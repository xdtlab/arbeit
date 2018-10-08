#!/usr/bin/python3

import sys
import subprocess
import asyncio
import aiohttp
import json
from pydaten.address import Address

DEFAULT_ENDPOINT = 'http://127.0.0.1:32323/mine'
DEFAULT_USERNAME = 'admin'
DEFAULT_PASSWORD = 'admin'

class Miner:

    def __init__(self, miner_path, url, username, password, address):
        self.address = address
        self.miner_process = None
        self.work_queue = asyncio.Queue()
        self.miner_path = miner_path
        self.loop = asyncio.get_event_loop()
        self.loop.run_until_complete(asyncio.gather(self.main(url, username, password)))

    async def main(self, url, username, password):
        session = aiohttp.ClientSession()
        url += '?username=' + username + '&password=' + password + '&address=' + str(self.address)
        async with session.ws_connect(url) as ws:
            await asyncio.gather(self.client(ws), self.miner(ws))

    def mine(self, data, target):
        self.miner_process = subprocess.Popen(self.miner_path, stdin = subprocess.PIPE, stdout = subprocess.PIPE)
        stdout, stderr = self.miner_process.communicate(data + target)
        return stdout

    async def miner(self, ws):
        while True:
            work = await self.work_queue.get()
            print("Working on work " + str(work['id']) + '!')
            future = self.loop.run_in_executor(None, self.mine, bytes.fromhex(work['data']), bytes.fromhex(work['target']))
            result = await future
            if result:
                print("Work " + str(work['id']) + ' done!')
                await ws.send_json({'id': work['id'], 'data': result.hex()})

    async def client(self, ws):
        async for msg in ws:
            data = json.loads(msg.data)
            if 'ok' not in data:
                if self.miner_process and self.miner_process.poll() is None:
                    print("Terminating!")
                    self.miner_process.terminate()
                await self.work_queue.put(data)
                if msg.type in (aiohttp.WSMsgType.CLOSED,
                                aiohttp.WSMsgType.ERROR):
                    break

MINER_PATH = './miner'

import argparse
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", help="Mining endpoint. (Default: {})".format(DEFAULT_ENDPOINT))
    parser.add_argument("--username", help="Username of the endpoint. (Default: {})".format(DEFAULT_USERNAME))
    parser.add_argument("--password", help="Password of the endpoint. (Default: {})".format(DEFAULT_PASSWORD))
    parser.add_argument("--address", help="Address of your wallet.")
    args = parser.parse_args()

    url = args.url or DEFAULT_ENDPOINT
    username = args.username or DEFAULT_USERNAME
    password = args.password or DEFAULT_PASSWORD
    address = args.address
    if address:
        try:
            miner = Miner(MINER_PATH, url, username, password, Address.from_string(address))
        except FileNotFoundError:
            print("Program \"{}\" not found. Try building the miner program with \"make\"!".format(MINER_PATH))
        except Exception as e:
            print("An error occurred:", e.message)
    else:
        print("Please specify your address!")
