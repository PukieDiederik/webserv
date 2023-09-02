# #!/usr/bin/ruby

require "cgi"

cgi = CGI.new

was_file_defeted = false

if File.exists?(cgi["filename"])
	was_file_defeted = true

	File.delete(cgi["filename"])
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
puts "						File #{was_file_defeted ? 'deleted' : 'not deleted'}"
puts '					</h1>'
puts '				</section>'
puts '			</div>'
puts '		</div>'
puts '	</body>'
puts '</html>'
