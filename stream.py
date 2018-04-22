import httplib
import urlparse


class MJPEGClient:
    """An iterable producing JPEG-encoded video frames from an MJPEG stream URL."""

    def __init__(self, url):
        self._url = urlparse.urlparse(url)

        h = httplib.HTTP(self._url.netloc)
        h.putrequest('GET', self._url.path)
        h.endheaders()
        errcode, errmsg, headers = h.getreply()

        if errcode == 200:
            ndx = headers["content-type"].find('=') + 1
            self._boundary = headers["content-type"][ndx:]
            self._fh = h.getfile()
        else:
            raise RuntimeError("HTTP %d: %s" % (errcode, errmsg))

    def __iter__(self):
        """Yields JPEG-encoded video frames."""

        # TODO: handle chunked encoding delimited by marker instead
        # of content-length.

        b = "--" + self._boundary
        print "boundary: %s" % b
        state = 1
        data = ""

        while True:
            length = None
            while True:
                line = self._fh.readline()

                if state == 1 and line == "\r\n":
                    state = 2
                elif state == 1:
                    if line.startswith("Content-Type: "):
                        type = line.split(" ")[1].strip()
                        print "%s" % type

                elif state == 2:
                    ndx = line.find(b)
                    if ndx == -1:
                        data = data + line
                    else:
                        data = data + line[0:ndx]
                        state = 1
                        break

            yield data
            data = ""


if __name__ == "__main__":
    from PIL import Image
    import StringIO
    import sys

    if len(sys.argv) != 3:
        print >> sys.stderr, "Usage: %s http://camera.example.com/mjpegstream outputfile" % sys.argv[0]
        raise SystemExit

    i = 1
    for jpegdata in MJPEGClient(sys.argv[1]):
        frame = Image.open(StringIO.StringIO(jpegdata))
        frame.save("%s-%d.jpeg" % (sys.argv[2], i))
        print "%d bytes, %dx%d pixels" % (
            len(jpegdata), frame.size[0], frame.size[1])
        i += 1
        if i == 81:
            break
