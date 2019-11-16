import dgram from 'dgram'
import { TV, BED, OFF, re, map, CONTROLLER_IP, CONTROLLER_PORT } from './constants.mjs'

const udpServer = dgram.createSocket('udp4')

export const watcher = {
  hasReply: false,
  statuses: new Map([
    [TV, OFF],
    [BED, OFF]
  ]),
  async waitForReply () {
    while (!this.hasReply) {
      await new Promise(resolve => setTimeout(resolve, 100))
    }
    this.hasReply = false
    return this.statuses
  },
  handleReplies (statuses) {
    for (const [, pin, status] of statuses) {
      const pinInt = parseInt(pin)
      if (this.statuses.has(pinInt)) this.statuses.set(pinInt, status)
    }
    this.hasReply = true
  }
}

udpServer.on('message', (msg, rinfo) => {
  msg = `${msg}`
  // console.log(`'${msg}' from ${rinfo.address}:${rinfo.port}`)
  const lines = msg.split('\n')
  const matches = lines.map(line => `${line}`.match(re)).filter(Boolean)
  if (matches.length > 0) {
    watcher.handleReplies(matches)
  }
})

udpServer.on('listening', () => {
  const { address, port } = udpServer.address()
  console.log(`UDP server listening ${address}:${port}`)
})

export const start = () => {
  udpServer.bind(9000)
  return udpServer
}

const send = packet =>
  udpServer.send(packet, 0, packet.length, CONTROLLER_PORT, CONTROLLER_IP, err => err && console.error(err))

export const status = pin => send(new Uint8Array([map[pin] || pin, 0]))
export const allStatus = () => send(new Uint8Array([0]))
export const off = pin => send(new Uint8Array([map[pin] || pin, 1]))
export const on = pin => send(new Uint8Array([map[pin] || pin, 2]))
