import http.client

# Connect to the server
conn = http.client.HTTPConnection('localhost:3000')

# Send the first request
conn.request('GET', '/HttpMessage.cpp')
response1 = conn.getresponse()
print('Response 1:', response1.status, response1.reason)
print(response1.read().decode())

# Send the second request
conn.request('GET', '/Server.cpp')
response2 = conn.getresponse()
print('Response 2:', response2.status, response2.reason)
print(response2.read().decode())

# Close the connection
conn.close()
