#!/usr/bin/env python
import asyncio
import websockets

async def handle_connection(websocket, path):
    print(f"Client connected to path: {path}")
    try:
        async for message in websocket:
            print(f"Received message: {message}")
            response = f"Server received: {message}"
            await websocket.send(response)
            print(f"Sent response: {response}")

    except websockets.exceptions.ConnectionClosed:
        print("Client disconnected")

# Replace the IP address with the address of the server you want to run
server_address = "192.168.36.172"
server_port = 80

start_server = websockets.serve(handle_connection, server_address, server_port)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
