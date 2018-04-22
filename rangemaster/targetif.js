function targetjs() {
  'use strict';

  let connections = {};
  let initialized = false;

  // Big endian read
  function readUInt32(abuff, offset) {
    var buffer = new Uint8Array(abuff);
    var value = 0;
    for (var i = offset; i < offset + 4; i++) {
      value = (value << 8) + buffer[i];
    }
    return value;
  }

  // Big endian write
  function writeUInt32(value, abuff, offset) {
    var buffer = new Uint8Array(abuff);
    for (var i = offset + 3; i >= offset; i--) {
      buffer[i] = value & 0xff;
      value = value >> 8;
    }
  }

  function append(abuff, data, offset) {
    var buffer = new Uint8Array(abuff);
    for (var i = 0; i < data.length; i++) {
      buffer[i + offset] = data[i];
    }
  }

  function connect(clientOptions, cbs) {
    let handle = connections[clientOptions.host + ':' + clientOptions.port];

    if (handle && handle.connected) {
      handle.ts = Date.now();
      // callback with asynchronous behavior.
      if (cbs.onOpen) {
        setTimeout(() => cbs.onOpen(handle), 0);
      }

      return;
    }

    if (clientOptions.type === 'tcp') {
      let socket = new clientOptions.constructor();

      if (!handle) {
        handle = {
          socket: socket,
          cbs: cbs,
          connected: false,
          clientOptions: clientOptions,
          ts: Date.now()
        };

        connections[clientOptions.host + ':' + clientOptions.port] = handle;
      }

      socket.connect(clientOptions.port, clientOptions.host, function() {
        console.log('Connected');
        connections[clientOptions.host + ':' + clientOptions.port] = 0;
        handle.connected = true;
        if (handle.cbs.onOpen) handle.cbs.onOpen(handle);
      });

      socket.on('close', function() {
        console.log('Disconnected');
        connections[clientOptions.host + ':' + clientOptions.port] = 0;
        handle.connected = false;
        if (handle.cbs.onClose) handle.cbs.onClose();
      });

      socket.on('end', function() {
        console.log('Ended');
        handle.connected = false;
      });

      socket.on('error', function() {
        console.log('Error on connection');
        handle.connected = false;
        if (handle.cbs.onError) handle.cbs.onError();
      });

      let size = 0;
      let readBuffer = Buffer.from([]);
      let sizeLen = 0;
      let readSize = true;
      socket.on('data', function(buffer) {
        try {
          if (readSize == true) {
            let obj = readVarint(buffer, size, sizeLen);
            readSize = obj.more;
            size = obj.value;
            sizeLen = obj.len;
            buffer = obj.buffer;
          }
          if (!readSize && buffer.size() > 0) {
            readBuffer.append(buffer);
            if (readBuffer.size() > readSize) console.log('oops');
            if (readBuffer.size() >= readSize) {
              if (handle.cbs.onMessage)
                handle.cbs.onMessage(handle, readBuffer);
              readSize = true;
              size = 0;
              sizeLen = 0;
              readBuffer = Buffer.from([]);
            }
          }
        } catch (err) {
          console.log('error: ', err);
          if (handle.cbs.onError) handle.cbs.onError();
        }
      });
    }
    // todo websocket
  }

  function send(handle, cmd, message) {
    sendVarint(handle, message.length + 1);
    sendVarint(handle, cmd & 0x7f); // ensure command is only one byte
    handle.socket.write(Buffer.from(message));
  }

  function sendVarint(handle, value) {
    while (value > 0) {
      let c = value & 0x7f;
      value = value >> 7;
      if (value > 0) c = c | 0x80;
      handle.socket.write(String.fromCharCode(c));
    }
  }

  function readVarint(buffer, value = 0, len = 0) {
    let more = true;
    let ndx = 0;
    while (ndx < buffer.length) {
      let c = buffer[ndx++];
      len++;
      value = ((c & 0x7f) << (len * 7)) + value;
    }
    return { more, value, buffer: buffer.slice(ndx), len };
  }

  return { connect, send };
}

module.exports = targetjs();
