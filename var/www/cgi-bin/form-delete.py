#!/usr/bin/python

import cgi
import os

try:
	form = cgi.FieldStorage()

	was_file_deleted = False

	if "filename" in form and form["filename"] and os.path.exists(form["filename"]):
		try:
			os.remove(form["filename"])
			was_file_deleted = True
		except Exception as e:
			was_file_deleted = False

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
	print("						File {}".format('deleted' if was_file_deleted else 'not deleted'))
	print('					</h1>')
	print('				</section>')
	print('			</div>')
	print('		</div>')
	print('	</body>')
	print('</html>')
except Exception as e:
	print("HTTP/1.0 500 Internal Server Error")
