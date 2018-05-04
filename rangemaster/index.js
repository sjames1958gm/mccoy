const fs = require('fs');
const path = require('path');
const express = require('express');
const morgan = require('morgan');
const cors = require('cors');
const bodyParser = require('body-parser');

const Target = require('./target');
require('dotenv').config();

let rootDir = process.argv[2];

let targets = [];

fs.readdir(rootDir, (err, files) => {
  if (err) {
    console.log(`Cannot open root directory: ${rootDir}`);
    return;
  }
  files.forEach(file => {
    if (/^target/.test(file)) {
      console.log(file);
      let targetPath = path.join(rootDir, file);
      let target = Target(targetPath);
      if (target) {
        targets.push(target);
      } else {
        console.log(`Target config failed for ${targetPath}`);
      }
    }
  });
});

function targetEvent(target, event) {
  console.log(`Event: ${target.id} -> ${event}`);
  targets.forEach(t => {
    console.log(`target ${t.id} is ${t.online ? 'online' : 'offline'}`);
  });
}

const app = express();
let port = process.env.PORT || 8080;
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(morgan('combined'));
app.use(cors('*'));

app.get('/targets', (req, res) => {
  res.json(targets.map(target => ({ id: target.id, state: target.state })));
});

app.get('/target/:id', (res, req) => {
  console.log(res.params.id);
  let id = res.params.id;
  let json = {};
  targets.some(t => {
    if (t.id == id) {
      json = { id: t.id, state: t.state, ip: t.ip, port: t.port };
      return true;
    }
    return false;
  });
  req.json(json);
});

app.post('/target/reset/:id', (res, req) => {
  console.log(res.params.id);
  let id = res.params.id;
  let json = {};
  targets.some(t => {
    if (t.id == id) {
      t.reset();
      return true;
    }
    return false;
  });
  req.json(json);
});

app.use(function(req, res) {
  res.sendStatus(404);
});

app.listen(port, function() {
  console.log('-------------------------------');
  console.log('server started on ' + port);
});
