from flask import Flask
from flask import render_template
from flask import request
from flask_cors import CORS
from os import environ

CAMERA_URL = environ.get('CAMERA_URL')

app = Flask(__name__)
CORS(app)

@app.route("/")
def main_page():
    return render_template('page.html', camera_url=CAMERA_URL)

@app.post("/infer")
def infer():
    data = request.get_data(False)
    print(data)
    return {
        'test': "Test"
    }

if __name__ == '__main__':
    app.run()