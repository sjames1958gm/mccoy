const net = require('net');
const dotenv = require('dotenv');
const path = require('path');
const fs = require('fs');
const targetif = require('./targetif.js');

const RESET = 1;
const PING = 2;
const CMDFILE = 3;
const HITLOG = 4;
const DEBUGLOG = 5;

const OFFLINE = 'offline';
const CONNECTED = 'connected';
const ONLINE = 'online';
const RUNNING = 'running';

let connections = {};

startClient = function(addr, port, cb, datacb) {
  console.log(`addr: ${addr} port: ${port}`);

  targetif.connect(
    { type: 'tcp', constructor: net.Socket, host: addr, port: port },
    {
      onOpen: handle => {
        if (cb) cb(null, handle);
      },
      onClose: () => {
        console.log(`xclose: ${addr}`);
        if (cb) cb('target is unavailable');
      },
      onError: () => {
        console.log(`xerror: ${addr}`);
        // if (cb) cb('target is unavailable');
      },
      onMessage: (h, cmd, message) => {
        // console.log(`onMessage: ${cmd} / ${message.length}`);
        if (datacb) datacb(h, cmd, message);
      }
    }
  );
};

function Target(targetDir) {
  console.log(targetDir);
  let configPath = path.join(targetDir, 'config.env');
  if (fs.existsSync(configPath)) {
    let config = dotenv.parse(fs.readFileSync(configPath));
    this.targetPath = path.resolve(targetDir);
    this.id = config.TARGETID;
    this.ip = config.TARGETIP;
    this.port = config.TARGETPORT;
    this.connected = false;
    this.state = OFFLINE;
    this.handlePing = handlePing;
    this.start = start;
    this.ping = ping;
    this.reset = reset;
    setTimeout(() => {
      this.start();
    }, 0);
  } else {
    console.log(`Could not read configuration at ${configPath}`);
  }
}

function start() {
  startClient(
    this.ip,
    this.port,
    (err, handle) => {
      if (err) {
        console.log(err);
        this.connected = false;
        this.state = OFFLINE;
        // this.eventcb(target, 'connectionerror');
        this.handle = 0;
        console.log(`Target: ${this.id} offline`);
        setTimeout(() => this.start(), 10000);
      } else {
        this.handle = handle;
        this.connected = true;
        this.state = CONNECTED;
        console.log(`Target: ${this.id} online`);
        this.reset();
        setTimeout(() => this.ping(), 2000);
      }
    },
    (handle, cmd, message) => {
      switch (cmd) {
        case PING:
          this.handlePing(message);
          break;
        default:
          console.log(`unknown command: ${cmd}`);
      }
    }
  );
}

function ping() {
  if (this.connected) {
    console.log(`Sending ping to ${this.id}`);
    targetif.send(this.handle, PING, 'Ping');
    setTimeout(() => this.ping(), 5000);
  } else {
    console.log('restart target');
    this.start();
  }
}

function handlePing() {
  console.log(`Ping received from ${this.id}`);
  this.state = ONLINE;
}

function reset() {
  if (this.connected) {
    // targetif.send(target.handle, RESET, '');
  }
}

module.exports = path => new Target(path);
