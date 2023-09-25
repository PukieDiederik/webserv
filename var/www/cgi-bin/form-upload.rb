#!/usr/bin/ruby

require "cgi"

cgi = CGI.new
params = cgi.params

was_file_uploaded = false

if params.has_key?"file"
	file = params["file"].first

	if file && file.original_filename
		server_file = ENV['UPLOAD_FOLDER'] + file.original_filename

		if (!File.directory?(server_file))
			File.open(server_file.untaint, "w") do |f|
				f << file.read
			end

			was_file_uploaded = true
		end
	end
end

puts 'HTTP/1.0 200 OK'
puts 'Content-type: text/html'
puts ''

puts '<!DOCTYPE html>'
puts '<html lang="en">'
puts '	<head>'
puts '		<title>42 Webserver - Form Submission</title>'
puts '		<link rel="stylesheet" href="/index.css">'
puts '		<script src="/index.js"></script>'
puts '	</head>'
puts '	<body>'
puts '		<div class="page-wrapper">'
puts '			<div class="container">'
puts '				<section>'
puts '					<h1 class="title">'
puts "						File #{was_file_uploaded ? 'uploaded' : 'not uploaded'}"
puts '					</h1>'
puts '				</section>'
puts '			</div>'
puts '		</div>'
puts '	</body>'
puts '</html>'
