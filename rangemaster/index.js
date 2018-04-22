const fs = require('fs');
const path = require('path');
const express = require('express');
const morgan = require('morgan');
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
      let target = Target(targetPath, targetEvent);
      if (target) {
        console.log(target);
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
app.use(morgan('tiny'));

app.use(function(req, res) {
  res.sendStatus(404);
});

app.listen(port, function() {
  console.log('-------------------------------');
  console.log('server started on ' + port);
});
