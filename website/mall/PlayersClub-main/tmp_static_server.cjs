const http = require('http');
const fs = require('fs');
const path = require('path');
const root = process.argv[2];
const port = Number(process.argv[3] || 4173);
const mime = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.svg': 'image/svg+xml',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.webp': 'image/webp',
  '.json': 'application/json; charset=utf-8',
  '.xml': 'application/xml; charset=utf-8'
};
http.createServer((req, res) => {
  const urlPath = decodeURIComponent((req.url || '/').split('?')[0]);
  let filePath = path.join(root, urlPath.replace(/^\//, ''));
  if (urlPath.endsWith('/')) filePath = path.join(root, urlPath.replace(/^\//, ''), 'index.html');
  if (!path.extname(filePath)) {
    const asFile = filePath + '.html';
    const asIndex = path.join(filePath, 'index.html');
    if (fs.existsSync(asFile)) filePath = asFile;
    else filePath = asIndex;
  }
  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.statusCode = 404;
      res.end('Not Found');
      return;
    }
    res.setHeader('Content-Type', mime[path.extname(filePath).toLowerCase()] || 'application/octet-stream');
    res.end(data);
  });
}).listen(port, '127.0.0.1', () => console.log(`Static server on http://127.0.0.1:${port}`));
