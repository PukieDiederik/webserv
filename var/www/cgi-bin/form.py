#!/usr/bin/python

import cgi

try:
    form = cgi.FieldStorage()

    print('HTTP/1.0 200 OK')
    print('Content-type: text/html')
    print('')

    print('<!DOCTYPE html>')
    print('<html lang="en">')
    print('	<head>')
    print('		<title>42 Webserver - Form Submission</title>')
    print('		<link rel="stylesheet" href="/index.css">')
    print('		<script src="/index.js"></script>')
    print('	</head>')
    print('	<body>')
    print('		<div class="page-wrapper">')
    print('			<div class="container">')
    print('				<section>')
    print('					<h1 class="title">')
    print('						Form Submission')
    print('					</h1>')
    print('					<p>')
    print("						First Name: {}".format(form.getvalue("fname", "")))
    print('						<br>')
    print("						Last Name: {}".format(form.getvalue("lname", "")))
    print('					</p>')
    print('				</section>')
    print('			</div>')
    print('		</div>')
    print('	</body>')
    print('</html>')
except Exception as e:
	print("HTTP/1.0 500 Internal Server Error")