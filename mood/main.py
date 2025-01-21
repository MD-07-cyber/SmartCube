from flask import Flask, request, jsonify
from deepface import DeepFace
import numpy as np
import cv2
from io import BytesIO
import threading

app = Flask(__name__)
responses = {}  # To store responses temporarily

# Expire old responses after a certain time (e.g., 60 seconds)
def cleanup_responses():
    import time
    while True:
        time.sleep(60)
        responses.clear()

# Start a thread to periodically clear old responses
threading.Thread(target=cleanup_responses, daemon=True).start()

@app.route('/mood', methods=['POST'])
def detect_mood():
    # Get the image data from the POST request
    img_bytes = request.data
    client_id = request.remote_addr  # Use client IP address as an identifier
    np_arr = np.frombuffer(img_bytes, np.uint8)
    img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

    # Ensure the image was loaded properly
    if img is None:
        return jsonify({'error': 'Image could not be decoded. Please check the image format.'})

    # Use DeepFace for emotion detection
    try:
        # DeepFace requires RGB format, so we convert the BGR image to RGB
        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        analysis = DeepFace.analyze(img_rgb, actions=['emotion'], enforce_detection=False)

        # Get the dominant emotion
        dominant_emotion = analysis[0]['dominant_emotion']
        response = {'mood': dominant_emotion}

        # Store the response temporarily
        responses[client_id] = response

        return jsonify(response)

    except Exception as e:
        return jsonify({'error': str(e)})

@app.route('/get-mood', methods=['GET'])
def get_mood():
    client_id = request.remote_addr  # Use the IP address of the second client
    if client_id in responses:
        return jsonify(responses[client_id])
    else:
        return jsonify({'error': 'No mood detected yet for this client.'}), 404

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)