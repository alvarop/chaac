FROM alvaroops/chaac-frontend-flask:latest

WORKDIR /usr/src/

CMD ["gunicorn", "--reload", "app:app", "--bind", "0.0.0.0:8000"]