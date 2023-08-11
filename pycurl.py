import requests

VERBOSE = False
counter = 0

def request_target():

    VERBOSE = False;

    if counter > 0:
        print('\n\n---------------------------------------------------------------------------------------\n\n')
        print('Custom server request: \n\n')

    host = 'localhost'

    print('Target host: ' + host)

    port = input('Target port: ')

    request = input('Request: ')

    target = input('Target: ')

    if request == 'GET':
        # GET request
        response = requests.get('http://localhost:' + port + '/' + target)

    if request == 'HEAD':
        # HEAD request
        response = requests.head('http://localhost:' + port + '/' + target)

    if VERBOSE:
        print('\nHEADER:\n')
        print(response.headers)
        print('\nBody:\n')
        print(response.text)

    print('Response status: ')
    print(response)
    print('\n')

    counter += 1

while 1:
    request_target()

