#!/usr/bin/python

import cgi
import os

try:
	form = cgi.FieldStorage()

	was_file_uploaded = False

	if "file" in form and form["file"].file:
		file_item = form["file"]

		try:
			with open(os.environ.get('UPLOAD_FOLDER') + file_item.filename, "wb") as file:
				was_file_uploaded = True

				while True:
					chunk = file_item.file.read(1024)
					if not chunk:
						break
					file.write(chunk)
		except Exception as e:
			was_file_uploaded = False

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
	print("						File {}".format('uploaded' if was_file_uploaded else 'not uploaded'))
	print('					</h1>')
	print('				</section>')
	print('			</div>')
	print('		</div>')
	print('	</body>')
	print('</html>')
except Exception as e:
	print("HTTP/1.0 500 Internal Server Error")