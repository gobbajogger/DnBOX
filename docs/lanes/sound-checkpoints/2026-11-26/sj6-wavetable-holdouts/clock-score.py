import json
m=json.load(open('metrics.json'))
for a,b in zip(m['test-sparse-32-7-8-1'],m['test-quarter-32-7-8-1']):
 print(a['wave'],a['note'],round(a['h1_deg_error'],8),round(b['h1_deg_error'],8),round(b['h1_deg_error']-a['h1_deg_error'],8))
