import Koa from 'koa'
import bodyParser from 'koa-bodyparser'
import koaStatic from 'koa-static'
import { map, mnere } from './constants.mjs'
import { allStatus, status, on, off, watcher, start } from './client.mjs'

const udpServer = start()

const app = new Koa()
app.use(bodyParser({ enableTypes: ['text'] }))
app.use(koaStatic('static'))
app.use(async ctx => {
  if (ctx.req.url !== '/') return

  const body = ctx.request.rawBody
  if (body === '?') {
    allStatus()
    const statuses = await watcher.waitForReply()
    ctx.body = Array.from(statuses.entries()).map(([pin, status]) => `${pin} ${status}`).join('\n')
    return
  }

  const matches = body.match(mnere)
  if (!matches) {
    ctx.status = 400
    return
  }

  const [, pin, action] = matches
  const pinInt = map[pin] || parseInt(pin, 10)

  if (action === 'on') on(pin)
  if (action === 'off') off(pin)
  if (action === '?') {
    status(pin)
    const statuses = await watcher.waitForReply()
    ctx.body = `${pin} ${statuses.get(pinInt)}`
  }

  ctx.status = 200
})
const server = app.listen(8000)

process.on('SIGINT', function () {
  console.log('Shutting down...')
  server.close()
  udpServer.close()
  process.exit(0)
})
