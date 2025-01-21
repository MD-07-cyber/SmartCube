from flask import Flask, request, jsonify
from deepface import DeepFace
import numpy as np
import cv2
import threading
from collections import defaultdict

app = Flask(__name__)

# Use a thread-safe dictionary for storing responses
responses = defaultdict(dict)

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
        # Preprocess image: Convert to RGB and resize for faster analysis
        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        img_resized = cv2.resize(img_rgb, (640, 480))  # Resize to 640x480 for efficiency

        # Analyze emotion using DeepFace
        analysis = DeepFace.analyze(img_resized, actions=['emotion'], enforce_detection=True)

        # Handle multiple faces
        if isinstance(analysis, list):
            # Get the dominant emotion of the first face detected
            dominant_emotion = analysis[0].get('dominant_emotion', 'unknown')
        else:
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