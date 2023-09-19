# #!/usr/bin/ruby

require "cgi"

cgi = CGI.new
params = cgi.params

if params.has_key?"file"
	file = params["file"].first
	server_file = ENV['UPLOAD_FOLDER'] + file.original_filename

	File.open(server_file.untaint, "w") do |f|
		f << file.read
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
puts '						Form Submission'
puts '					</h1>'
puts '				</section>'
puts '			</div>'
puts '		</div>'
puts '	</body>'
puts '</html>'
