#!/usr/bin/env python3
"""TinyML data collection and training pipeline for ESP32-S3.

This script supports three practical steps on the PC:

1. Collect labeled samples into a CSV file.
2. Train a very small MLP with TensorFlow/Keras.
3. Export the model as .h5, .tflite, and a C header for TFLM firmware.

Dataset format:
    temperature,humidity,label

Label mapping:
    0 = No action
    1 = Turn on fan
    2 = Need watering

Examples:
    python scripts/tinyml_pipeline.py collect --csv data/samples.csv
    python scripts/tinyml_pipeline.py train --csv data/samples.csv
    python scripts/tinyml_pipeline.py train --csv data/samples.csv \
        --out-dir ml_artifacts --c-header include/tinyml_model_data.h
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
from typing import Dict, Iterator, List, Tuple


LABEL_MAP: Dict[int, str] = {
    0: "No action",
    1: "Turn on fan",
    2: "Need watering",
}

LABEL_ALIASES: Dict[str, int] = {
    "0": 0,
    "1": 1,
    "2": 2,
    "no_action": 0,
    "no action": 0,
    "fan": 1,
    "turn_on_fan": 1,
    "turn on fan": 1,
    "watering": 2,
    "need_watering": 2,
    "need watering": 2,
}


def require_numpy():
    try:
        import numpy as np
    except ImportError as exc:
        raise SystemExit(
            "Missing dependency: numpy. Install it before running training, "
            "for example: pip install numpy"
        ) from exc
    return np


def require_tensorflow():
    try:
        import tensorflow as tf
    except ImportError as exc:
        raise SystemExit(
            "Missing dependency: tensorflow. Install it before running training, "
            "for example: pip install tensorflow"
        ) from exc
    return tf


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Collect data and train a TinyML MLP for ESP32-S3."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    collect_parser = subparsers.add_parser(
        "collect",
        help="Interactively append temperature/humidity/label samples to a CSV file.",
    )
    collect_parser.add_argument(
        "--csv",
        type=Path,
        default=Path("data/tinyml_samples.csv"),
        help="Path to the dataset CSV file.",
    )

    train_parser = subparsers.add_parser(
        "train",
        help="Train the MLP, save .h5, convert to .tflite, and export a C header.",
    )
    train_parser.add_argument(
        "--csv",
        type=Path,
        default=Path("data/tinyml_samples.csv"),
        help="Path to the dataset CSV file.",
    )
    train_parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("ml_artifacts"),
        help="Directory used for generated model artifacts.",
    )
    train_parser.add_argument(
        "--c-header",
        type=Path,
        default=Path("include/tinyml_model_data.h"),
        help="Header file generated for firmware embedding.",
    )
    train_parser.add_argument(
        "--epochs",
        type=int,
        default=150,
        help="Maximum number of training epochs.",
    )
    train_parser.add_argument(
        "--batch-size",
        type=int,
        default=16,
        help="Mini-batch size for training.",
    )
    train_parser.add_argument(
        "--validation-split",
        type=float,
        default=0.2,
        help="Validation split ratio in range (0, 1).",
    )
    train_parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Random seed for dataset shuffling and training.",
    )

    return parser.parse_args()


def ensure_csv_file(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists():
        with path.open("w", newline="", encoding="utf-8") as file:
            writer = csv.writer(file)
            writer.writerow(["temperature", "humidity", "label"])


def normalize_label(value: str) -> int:
    key = value.strip().lower()
    if key in LABEL_ALIASES:
        return LABEL_ALIASES[key]

    raise ValueError(
        "Invalid label. Use 0, 1, 2 or aliases: "
        "no_action, fan, need_watering."
    )


def collect_samples(csv_path: Path) -> None:
    ensure_csv_file(csv_path)
    print(f"Writing samples to: {csv_path}")
    print("Press Enter on temperature to stop.")
    print("Label mapping:")
    for label, name in LABEL_MAP.items():
        print(f"  {label} = {name}")

    while True:
        temp_raw = input("Temperature (C): ").strip()
        if not temp_raw:
            print("Stopped collecting samples.")
            return

        humi_raw = input("Humidity (%): ").strip()
        label_raw = input("Label (0/1/2 or alias): ").strip()

        try:
            temperature = float(temp_raw)
            humidity = float(humi_raw)
            label = normalize_label(label_raw)
        except ValueError as exc:
            print(f"Skipped sample: {exc}")
            continue

        with csv_path.open("a", newline="", encoding="utf-8") as file:
            writer = csv.writer(file)
            writer.writerow([temperature, humidity, label])

        print(
            f"Saved sample -> temperature={temperature:.2f}, "
            f"humidity={humidity:.2f}, label={label} ({LABEL_MAP[label]})"
        )


def load_dataset(csv_path: Path) -> Tuple[np.ndarray, np.ndarray]:
    np = require_numpy()

    if not csv_path.exists():
        raise FileNotFoundError(f"Dataset file not found: {csv_path}")

    features: List[List[float]] = []
    labels: List[int] = []

    with csv_path.open("r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        required = {"temperature", "humidity", "label"}
        if not required.issubset(reader.fieldnames or set()):
            raise ValueError(
                "CSV must contain columns: temperature, humidity, label"
            )

        for row_index, row in enumerate(reader, start=2):
            try:
                temperature = float(row["temperature"])
                humidity = float(row["humidity"])
                label = normalize_label(row["label"])
            except Exception as exc:  # pragma: no cover - defensive validation
                raise ValueError(
                    f"Invalid data at row {row_index}: {exc}"
                ) from exc

            features.append([temperature, humidity])
            labels.append(label)

    if not features:
        raise ValueError("Dataset is empty.")

    unique_labels = sorted(set(labels))
    if unique_labels != [0, 1, 2]:
        raise ValueError(
            "Dataset must contain all labels 0, 1, and 2 at least once."
        )

    return np.asarray(features, dtype=np.float32), np.asarray(labels, dtype=np.int32)


def split_dataset(
    features: np.ndarray,
    labels: np.ndarray,
    validation_split: float,
    seed: int,
) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    np = require_numpy()

    if not 0.0 < validation_split < 1.0:
        raise ValueError("validation_split must be between 0 and 1.")

    if len(features) < 10:
        raise ValueError("Dataset is too small. Collect at least 10 samples.")

    rng = np.random.default_rng(seed)
    indices = rng.permutation(len(features))
    val_size = max(1, int(len(features) * validation_split))

    if len(features) - val_size < 3:
        raise ValueError(
            "Training set would be too small after validation split."
        )

    val_indices = indices[:val_size]
    train_indices = indices[val_size:]

    return (
        features[train_indices],
        labels[train_indices],
        features[val_indices],
        labels[val_indices],
    )


def compute_normalization(train_features: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    np = require_numpy()

    mean = train_features.mean(axis=0)
    std = train_features.std(axis=0)
    std = np.where(std < 1e-6, 1.0, std)
    return mean.astype(np.float32), std.astype(np.float32)


def standardize(
    features: np.ndarray,
    mean: np.ndarray,
    std: np.ndarray,
) -> np.ndarray:
    np = require_numpy()
    return ((features - mean) / std).astype(np.float32)


def build_model(seed: int):
    tf = require_tensorflow()

    tf.keras.utils.set_random_seed(seed)

    model = tf.keras.Sequential(
        [
            tf.keras.layers.Input(shape=(2,), name="sensor_input"),
            tf.keras.layers.Dense(8, activation="relu", name="dense_1"),
            tf.keras.layers.Dense(8, activation="relu", name="dense_2"),
            tf.keras.layers.Dense(3, activation="softmax", name="class_output"),
        ]
    )
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.01),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"],
    )
    return model


def representative_dataset(
    normalized_features: np.ndarray,
) -> Iterator[List[np.ndarray]]:
    sample_count = min(len(normalized_features), 100)
    for row in normalized_features[:sample_count]:
        yield [row.reshape(1, 2).astype(np.float32)]


def convert_to_tflite_int8(
    keras_h5_path: Path,
    rep_features: np.ndarray,
    tflite_path: Path,
) -> Dict[str, float]:
    tf = require_tensorflow()

    model = tf.keras.models.load_model(keras_h5_path)
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = lambda: representative_dataset(rep_features)
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    tflite_model = converter.convert()
    tflite_path.write_bytes(tflite_model)

    interpreter = tf.lite.Interpreter(model_content=tflite_model)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()[0]
    output_details = interpreter.get_output_details()[0]

    return {
        "input_scale": float(input_details["quantization"][0]),
        "input_zero_point": int(input_details["quantization"][1]),
        "output_scale": float(output_details["quantization"][0]),
        "output_zero_point": int(output_details["quantization"][1]),
    }


def bytes_to_c_array(data: bytes, values_per_line: int = 12) -> str:
    hex_values = [f"0x{byte:02x}" for byte in data]
    lines: List[str] = []
    for index in range(0, len(hex_values), values_per_line):
        lines.append("  " + ", ".join(hex_values[index : index + values_per_line]))
    return ",\n".join(lines)


def export_c_header(
    tflite_path: Path,
    header_path: Path,
    mean: np.ndarray,
    std: np.ndarray,
    quant_info: Dict[str, float],
) -> None:
    header_path.parent.mkdir(parents=True, exist_ok=True)
    model_bytes = tflite_path.read_bytes()
    c_array = bytes_to_c_array(model_bytes)
    guard = header_path.stem.upper() + "_H"

    content = f"""#ifndef {guard}
#define {guard}

#include <cstddef>
#include <cstdint>

// Generated by scripts/tinyml_pipeline.py
// Label mapping:
//   0 = No action
//   1 = Turn on fan
//   2 = Need watering
//
// In firmware, normalize raw inputs before quantization:
//   norm_temp = (temperature - kInputMean[0]) / kInputStd[0]
//   norm_humi = (humidity    - kInputMean[1]) / kInputStd[1]
// Then map to int8 using:
//   q = round(normalized / kInputScale) + kInputZeroPoint

namespace tinyml {{

constexpr std::size_t kModelLength = {len(model_bytes)};
constexpr float kInputMean[2] = {{{mean[0]:.8f}f, {mean[1]:.8f}f}};
constexpr float kInputStd[2] = {{{std[0]:.8f}f, {std[1]:.8f}f}};
constexpr float kInputScale = {quant_info["input_scale"]:.10f}f;
constexpr int kInputZeroPoint = {quant_info["input_zero_point"]};
constexpr float kOutputScale = {quant_info["output_scale"]:.10f}f;
constexpr int kOutputZeroPoint = {quant_info["output_zero_point"]};

alignas(16) const unsigned char kModelData[] = {{
{c_array}
}};

}}  // namespace tinyml

#endif  // {guard}
"""
    header_path.write_text(content, encoding="utf-8")


def evaluate_predictions(
    probabilities: np.ndarray,
    labels: np.ndarray,
) -> float:
    np = require_numpy()
    predictions = np.argmax(probabilities, axis=1)
    return float(np.mean(predictions == labels))


def print_dataset_summary(features: np.ndarray, labels: np.ndarray) -> None:
    np = require_numpy()
    print(f"Loaded {len(features)} samples from dataset.")
    for label, name in LABEL_MAP.items():
        count = int(np.sum(labels == label))
        print(f"  label {label} ({name}): {count} samples")


def train_pipeline(args: argparse.Namespace) -> None:
    tf = require_tensorflow()

    features, labels = load_dataset(args.csv)
    print_dataset_summary(features, labels)

    (
        train_features,
        train_labels,
        val_features,
        val_labels,
    ) = split_dataset(features, labels, args.validation_split, args.seed)

    mean, std = compute_normalization(train_features)
    train_x = standardize(train_features, mean, std)
    val_x = standardize(val_features, mean, std)

    model = build_model(args.seed)
    callbacks = [
        tf.keras.callbacks.EarlyStopping(
            monitor="val_loss",
            patience=20,
            restore_best_weights=True,
        )
    ]

    history = model.fit(
        train_x,
        train_labels,
        validation_data=(val_x, val_labels),
        epochs=args.epochs,
        batch_size=args.batch_size,
        verbose=0,
        callbacks=callbacks,
    )

    train_accuracy = evaluate_predictions(model.predict(train_x, verbose=0), train_labels)
    val_accuracy = evaluate_predictions(model.predict(val_x, verbose=0), val_labels)

    args.out_dir.mkdir(parents=True, exist_ok=True)
    h5_path = args.out_dir / "tinyml_action_model.h5"
    tflite_path = args.out_dir / "tinyml_action_model_int8.tflite"
    summary_path = args.out_dir / "tinyml_training_summary.json"

    model.save(h5_path)
    quant_info = convert_to_tflite_int8(h5_path, train_x, tflite_path)
    export_c_header(tflite_path, args.c_header, mean, std, quant_info)

    summary = {
        "dataset_csv": str(args.csv),
        "sample_count": int(len(features)),
        "train_sample_count": int(len(train_features)),
        "validation_sample_count": int(len(val_features)),
        "train_accuracy": train_accuracy,
        "validation_accuracy": val_accuracy,
        "epochs_ran": len(history.history["loss"]),
        "normalization_mean": [float(mean[0]), float(mean[1])],
        "normalization_std": [float(std[0]), float(std[1])],
        "label_mapping": LABEL_MAP,
        "h5_model": str(h5_path),
        "tflite_model": str(tflite_path),
        "c_header": str(args.c_header),
        "quantization": quant_info,
    }
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    print()
    print("Training complete.")
    print(f"  .h5 saved to      : {h5_path}")
    print(f"  .tflite saved to  : {tflite_path}")
    print(f"  C header saved to : {args.c_header}")
    print(f"  Summary saved to  : {summary_path}")
    print(f"  Train accuracy    : {train_accuracy:.4f}")
    print(f"  Val accuracy      : {val_accuracy:.4f}")
    print(f"  Input mean        : {mean.tolist()}")
    print(f"  Input std         : {std.tolist()}")
    print(
        "  Input quant       : "
        f"scale={quant_info['input_scale']:.10f}, "
        f"zero_point={quant_info['input_zero_point']}"
    )


def main() -> None:
    args = parse_args()

    if args.command == "collect":
        collect_samples(args.csv)
        return

    if args.command == "train":
        train_pipeline(args)
        return

    raise ValueError(f"Unsupported command: {args.command}")


if __name__ == "__main__":
    main()
