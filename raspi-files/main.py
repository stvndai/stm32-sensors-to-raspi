import asyncio
import smbus2
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles

I2C_BUS = 1
STM32_ADDR = 0x28
POLL_INTERVAL = 1.0

app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")

bus = smbus2.SMBus(I2C_BUS)
connected_clients: set[WebSocket] = set()


def read_dht_data():
    try:
        data = bus.read_i2c_block_data(STM32_ADDR, 0x00, 5)
        humidity_int, humidity_dec, temp_int, temp_dec, checksum = data
        expected = (humidity_int + humidity_dec + temp_int + temp_dec) & 0xFF

        return {
            "humidity": humidity_int + humidity_dec / 10,
            "temperature": temp_int + temp_dec / 10,
            "checksum_valid": checksum == expected,
        }
    except OSError:
        return None


@app.get("/")
async def root():
    with open("static/index.html") as f:
        return HTMLResponse(f.read())


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    connected_clients.add(websocket)
    try:
        while True:
            await websocket.receive_text()  # keeps connection alive, ignores client messages
    except WebSocketDisconnect:
        connected_clients.remove(websocket)


async def poll_loop():
    while True:
        reading = read_dht_data()
        if reading and reading["checksum_valid"]:
            message = {
                "temperature": reading["temperature"],
                "humidity": reading["humidity"],
            }
            disconnected = set()
            for client in connected_clients:
                try:
                    await client.send_json(message)
                except Exception:
                    disconnected.add(client)
            connected_clients.difference_update(disconnected)
        await asyncio.sleep(POLL_INTERVAL)


@app.on_event("startup")
async def startup_event():
    asyncio.create_task(poll_loop())