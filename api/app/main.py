from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import json
import requests
import os
import sqlite3
import logging
from redis import Redis
from app.config import settings

GOOGLE_API_KEY = os.getenv("GOOGLE_API_KEY")

app = FastAPI()

# Set up database
con = sqlite3.connect(':memory:')

# Setup CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allow all origins for simplicity. Adjust as needed.
    allow_credentials=True,
    allow_methods=["*"],  # Allow all methods
    allow_headers=["*"],  # Allow all headers
)

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Initialize Redis
redis_client = Redis(host=settings.REDIS_HOST, port=settings.REDIS_PORT)


@app.on_event("startup")
async def startup_event():
    """Creates an in-memory database with a user table, and populate it with
    one account"""
    cur = con.cursor()
    cur.execute('''CREATE TABLE users (email text, password text)''')
    cur.execute('''INSERT INTO users VALUES ('me@me.com', '123456')''')
    con.commit()

class UserLogin(BaseModel):
    email: str
    password: str

class Location(BaseModel):
    latitude: float
    longitude: float

@app.post("/login")
async def login(user: UserLogin):
    logger.info(f"Login attempt for email: {user.email}")
    cur = con.cursor()
    # SQL injection!
    cur.execute("SELECT * FROM users WHERE email = '%s' and password = '%s'" % (user.email, user.password))
    if cur.fetchone() is not None:
        logger.info(f"Login successful for email: {user.email}")
        return {"message": "Login successful"}
    else: 
        logger.info(f"Login failed for email: {user.email}")
        raise HTTPException(status_code=401, detail="Invalid credentials")
    
@app.post("/logout")
async def logout(user: UserLogin):
    logger.info(f"Logout for email: {user.email}")
    return {"message": "Logout successful"}

@app.post("/location")
async def receive_location(location: Location):
    location_data = {'latitude': location.latitude, 'longitude': location.longitude}
    redis_client.rpush('locations', json.dumps(location_data))
    return {"message": "Location received"}

@app.get("/locations")
async def get_locations():
    locations = [json.loads(loc) for loc in redis_client.lrange('locations', 0, -1)]
    return {"locations": locations}
