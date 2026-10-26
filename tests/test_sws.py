#!/usr/bin/env python3
import http.client, os, socket, subprocess, time, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PORT=18991
class TestSWS(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.p=subprocess.Popen([str(ROOT/'sws'),'-d','-i','127.0.0.1','-p',str(PORT),'-c',str(ROOT/'cgi-bin'),str(ROOT/'www')],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  for _ in range(40):
   try:
    s=socket.create_connection(('127.0.0.1',PORT),.15);s.close();break
   except OSError:time.sleep(.1)
 @classmethod
 def tearDownClass(cls):cls.p.terminate();cls.p.wait(timeout=3)
 def req(self,method,path,headers=None):
  c=http.client.HTTPConnection('127.0.0.1',PORT,timeout=4);c.request(method,path,headers=headers or {});r=c.getresponse();body=r.read();v=(r.status,dict(r.getheaders()),body);c.close();return v
 def test_get(self):self.assertIn(b'CS631',self.req('GET','/')[2])
 def test_head(self):
  code,h,b=self.req('HEAD','/docs/hello.txt');self.assertEqual(code,200);self.assertEqual(b,b'');self.assertGreater(int(h['Content-Length']),0)
 def test_404(self):self.assertEqual(self.req('GET','/does-not-exist')[0],404)
 def test_304(self):self.assertEqual(self.req('GET','/docs/hello.txt',{'If-Modified-Since':'Wed, 01 Jan 2100 00:00:00 GMT'})[0],304)
 def test_directory(self):self.assertIn(b'hello.txt',self.req('GET','/docs/')[2])
 def test_cgi(self):self.assertIn(b'CGI is working',self.req('GET','/cgi-bin/hello.sh?name=CS631')[2])
 def test_http11(self):self.assertEqual(self.req('GET','/')[0],200)
 def test_headers(self):
  c,h,b=self.req('GET','/docs/hello.txt');self.assertEqual(c,200)
  for k in ('Date','Server','Last-Modified','Content-Type','Content-Length'):self.assertIn(k,h)
 def test_method(self):self.assertEqual(self.req('POST','/')[0],501)
 def test_traversal(self):self.assertIn(self.req('GET','/..%2fetc/passwd')[0],(400,403,404))
if __name__=='__main__':unittest.main()
