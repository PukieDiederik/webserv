# #!/usr/bin/ruby

require "cgi"

cgi = CGI.new
params = cgi.params

puts "HTTP/1.0 200 OK"
puts "Content-type: text/html"
puts ""
puts "<html><body>This is a test</body></html>"
puts "<p>params: "
puts params
puts "</p>"
puts "</body></html>"
