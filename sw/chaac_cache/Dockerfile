FROM python:3-slim

WORKDIR /usr/src/

COPY requirements.txt ./

RUN pip install --no-cache-dir -r requirements.txt

COPY chaac_cache.py .

CMD ["python", "-B", "-u", "./chaac_cache.py"]
