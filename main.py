

from flask import Flask, request, jsonify
from deepface import DeepFace
import numpy as np
import cv2
import threading
from collections import defaultdict
import mediapipe as mp

app = Flask(__name__)

# Initialize Mediapipe Face Detection
mp_face_detection = mp.solutions.face_detection
face_detection = mp_face_detection.FaceDetection(min_detection_confidence=0.5)

# Thread-safe dictionary for storing responses
responses = defaultdict(dict)

# Background thread to clean up old responses
def cleanup_responses():
    import time
    while True:
        time.sleep(300)  # Cleanup every 5 minutes
        responses.clear()

threading.Thread(target=cleanup_responses, daemon=True).start()

@app.route('/mood', methods=['POST'])
def detect_mood():
    client_id = request.args.get('client_id', 'unknown_client')
    img_bytes = request.data

    if not img_bytes:
        return jsonify({'error': 'No image data received'}), 400

    # Decode image from bytes
    np_arr = np.frombuffer(img_bytes, np.uint8)
    img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
    if img is None:
        return jsonify({'error': 'Failed to decode image'}), 400

    try:
        # Convert to RGB for Mediapipe
        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        results = face_detection.process(img_rgb)

        if not results.detections:
            return jsonify({'error': 'No faces detected'}), 404

        # Process the first detected face
        for detection in results.detections:
            bboxC = detection.location_data.relative_bounding_box
            ih, iw, _ = img.shape
            x, y, w, h = int(bboxC.xmin * iw), int(bboxC.ymin * ih), int(bboxC.width * iw), int(bboxC.height * ih)

            # Ensure valid coordinates
            x, y = max(0, x), max(0, y)
            face_crop = img[y:y + h, x:x + w]

            # Resize face for DeepFace analysis
            if face_crop.size == 0:
                return jsonify({'error': 'Failed to crop face'}), 400
            face_resized = cv2.resize(face_crop, (224, 224))  # DeepFace works well with 224x224

            # Convert to RGB for DeepFace
            face_rgb = cv2.cvtColor(face_resized, cv2.COLOR_BGR2RGB)

            # Analyze emotion using DeepFace
            analysis = DeepFace.analyze(face_rgb, actions=['emotion'], enforce_detection=False)

            # Check if analysis returned a list (multiple faces detected)
            if isinstance(analysis, list):
                # Extract dominant emotion from the first face detected
                dominant_emotion = analysis[0].get('dominant_emotion', 'unknown')
            else:
                # Extract dominant emotion from a single-face result
                dominant_emotion = analysis.get('dominant_emotion', 'unknown')

            response = {'mood': dominant_emotion}
            responses[client_id] = response  # Save response for the client
            return jsonify(response)

    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/get-mood', methods=['GET'])
def get_mood():
    client_id = request.args.get('client_id', 'unknown_client')
    if client_id in responses:
        return jsonify(responses[client_id])
    else:
        return jsonify({'error': 'No mood detected yet or response expired.'}), 404

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
