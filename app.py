import os
from flask import Flask, request, jsonify
from flask_cors import CORS
import tensorflow as tf
from tensorflow.keras.models import load_model
import numpy as np
from PIL import Image
import io

app = Flask(__name__)
CORS(app)

# Load the trained Keras model once at startup
model = load_model("component_model.h5")

# Define the class labels (ensure these match your model's training order)
class_labels = ["capacitor", "led", "relay", "resistor", "sevensegment", "switch"]

# Function to preprocess image for model prediction
def preprocess_image(image_bytes):
    img = Image.open(io.BytesIO(image_bytes)).convert("RGB")
    img = img.resize((128, 128))  # Resize to model's expected input size
    img_array = np.array(img) / 255.0  # Normalize to [0, 1]
    return np.expand_dims(img_array, axis=0)  # Add batch dimension

# Endpoint for image classification
@app.route("/predict", methods=["POST"])
def predict():
    if "image" not in request.files:
        return jsonify({"error": "No image file provided"}), 400

    image_file = request.files["image"]
    img_data = image_file.read()

    try:
        processed = preprocess_image(img_data)
        predictions = model.predict(processed)[0]
        class_idx = np.argmax(predictions)
        label = class_labels[class_idx]
        confidence = float(predictions[class_idx])
        return jsonify({"label": label, "confidence": confidence})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True)
