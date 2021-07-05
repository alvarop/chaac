FROM python:3-slim

WORKDIR /usr/src/app

COPY requirements.txt ./

RUN pip install --no-cache-dir -r requirements.txt

copy chaac_logger.py .

CMD ["python", "-B", "-u", "./chaac_logger.py", "--db", "/chaac/chaac.db" ]
