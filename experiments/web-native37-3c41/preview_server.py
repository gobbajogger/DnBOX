"""Internal managed preview only. No device, authentication, guest memory or publication API."""
from pathlib import Path
from http.server import ThreadingHTTPServer,SimpleHTTPRequestHandler
import json,os,argparse
from native_session import NativeSession
BASE=Path(__file__).resolve().parent;ROOT=BASE/'browser-shell'
STATIC_PATHS={
 '/':'index.html','/index.html':'index.html','/app.js':'app.js','/style.css':'style.css','/surface.mjs':'surface.mjs','/held-input.mjs':'held-input.mjs','/control-actions.mjs':'control-actions.mjs',
 '/assets/Barlow-OFL.txt':'assets/Barlow-OFL.txt',
 '/assets/BarlowSemiCondensed-Medium.ttf':'assets/BarlowSemiCondensed-Medium.ttf',
 '/assets/BarlowSemiCondensed-Bold.ttf':'assets/BarlowSemiCondensed-Bold.ttf'}
def static_asset(path):
 if path=='/browser-input/input.mjs':return BASE/'browser-input/input.mjs'
 if path not in STATIC_PATHS:raise ValueError('not a supported static asset')
 return ROOT/STATIC_PATHS[path]
def handle_event(payload,session):
 if type(payload) is not dict or set(payload)!={'command','args'} or type(payload['command']) is not str or type(payload['args']) is not list:raise ValueError('invalid whitelisted event envelope')
 frame=session.event(payload['command'],*payload['args'])
 frame.pop('sections',None)
 return frame
class Handler(SimpleHTTPRequestHandler):
 def __init__(self,*a,**kw):super().__init__(*a,directory=str(ROOT),**kw)
 def translate_path(self,path):return str(static_asset(path))
 def do_GET(self):
  try:static_asset(self.path)
  except ValueError:self.send_error(404);return
  super().do_GET()
 def do_HEAD(self):
  try:static_asset(self.path)
  except ValueError:self.send_error(404);return
  super().do_HEAD()
 def end_headers(self):
  self.send_header('Cache-Control','no-store');self.send_header('X-Content-Type-Options','nosniff')
  self.send_header('Content-Security-Policy',"default-src 'self'; style-src 'self' 'unsafe-inline'; script-src 'self'; connect-src 'self'; font-src 'self'; object-src 'none'; frame-ancestors 'self'")
  super().end_headers()
 def do_POST(self):
  if self.path!='/native-event':self.send_error(404);return
  if self.headers.get('X-Dnbox-Preview')!='native-host' or self.headers.get_content_type()!='application/json':self.send_error(403);return
  try:
   n=int(self.headers.get('Content-Length','0'))
   if not 0<n<=2048:raise ValueError('request too large')
   payload=json.loads(self.rfile.read(n));result=handle_event(payload,self.server.native_session);status=200
  except (ValueError,TypeError) as e:result={'error':str(e)};status=400
  except Exception:result={'error':'Native host stopped or parity check failed. No device is affected.'};status=500
  data=json.dumps(result,separators=(',',':')).encode();self.send_response(status);self.send_header('Content-Type','application/json');self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=int(os.environ.get('PORT','3000')));p.add_argument('--host',default='0.0.0.0');args,_=p.parse_known_args()
 server=ThreadingHTTPServer((args.host,args.port),Handler);server.native_session=NativeSession()
 print('Internal source-native preview listening on managed port '+str(args.port),flush=True)
 try:server.serve_forever()
 finally:server.native_session.close();server.server_close()
