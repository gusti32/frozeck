from flask import Flask
from flask import render_template

app = Flask(__name__)

@app.route("/")
def main_page():
    return render_template('page.html')

@app.route("/infer")
def infer():
    return 'Test'

if __name__ == '__main__':
    app.run()