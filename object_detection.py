# detect_and_classify.py

from ultralytics import YOLO
from tensorflow.keras.models import load_model
import cv2
import numpy as np

# Load models
detector = YOLO('best.pt')  # YOLOv8 model
classifier = load_model('component_classifier_model.h5')  # CNN model

# Define class names for classifier
class_names = ['Capacitor', 'LED', 'Relay', 'Resistor', 'SevenSegment', 'Switch']  # Update if needed

def detect_and_classify(image_path):
    # Load image
    img = cv2.imread(image_path)

    # Run detection
    results = detector(img)

    output = []
    for box in results[0].boxes:
        # Get coordinates
        x1, y1, x2, y2 = map(int, box.xyxy[0].tolist())

        # Crop component
        crop = img[y1:y2, x1:x2]

        # Preprocess crop for classifier
        crop_resized = cv2.resize(crop, (64, 64))  # Change if your classifier input size is different
        crop_normalized = crop_resized / 255.0
        crop_input = np.expand_dims(crop_normalized, axis=0)

        # Predict class
        pred = classifier.predict(crop_input, verbose=0)
        label = class_names[np.argmax(pred)]

        output.append({
            'label': label,
            'confidence': float(np.max(pred)),
            'bbox': [x1, y1, x2, y2]
        })

    return output
