FROM python:3-slim

WORKDIR /usr/src/

COPY requirements.txt ./

RUN pip install --no-cache-dir -r requirements.txt

COPY ./app /usr/src/app

CMD ["gunicorn","--workers","5","--bind","0.0.0.0:12345","-m","007","app:app"]
