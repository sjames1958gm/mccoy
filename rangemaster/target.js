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
      onMessage: (h, message) => {
        if (datacb) datacb(h, message);
      }
    }
  );
};

function start(target) {
  startClient(
    target.ip,
    target.port,
    (err, handle) => {
      if (err) {
        console.log(err);
        target.online = false;
        target.eventcb(target, 'connectionerror');
        target.handle = 0;
        console.log(`Target: ${target.id} offline`);
      } else {
        target.handle = handle;
        target.online = true;
        console.log(`Target: ${target.id} online`);
        target.eventcb(target, 'connected');
        resetTarget(target);
        setTimeout(() => pingTarget(target), 2000);
      }
    },
    (handle, message) => {
      console.log(message);
    }
  );
}

function Target(targetDir, eventcb) {
  console.log(targetDir);
  let configPath = path.join(targetDir, 'config.env');
  if (fs.existsSync(configPath)) {
    let config = dotenv.parse(fs.readFileSync(configPath));
    let target = {
      targetPath: path.resolve(targetDir),
      id: config.TARGETID,
      ip: config.TARGETIP,
      port: config.TARGETPORT,
      online: false,
      eventcb
    };
    setTimeout(() => {
      eventcb(target, 'created'), start(target);
    }, 0);
    return target;
  } else {
    console.log(`Could not read configuration at ${configPath}`);
  }
}

function pingTarget(target) {
  if (target.online) {
    if (target.handle == 0) {
      console.log('??');
      return;
    }
    targetif.send(target.handle, PING, '');
    setTimeout(() => pingTarget(target), 5000);
  } else {
    console.log('restart target');
    start(target);
  }
}

function resetTarget(target) {
  if (target.online) {
    if (target.handle == 0) {
      console.log('??');
      return;
    }
    targetif.send(target.handle, RESET, '');
  }
}

module.exports = Target;
