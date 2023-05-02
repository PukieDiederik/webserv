# How to install
`make` will compile the code into an executable (`webserv`) which you can run using:
```bash
./webserv [configuration file]
```
If no configuration file is provided it will use the default configuration file.

# Contributing
1) Fork this repository
2) Make a branch on your fork with the following format (without the square brackets) `[name]-[subject]` (`git checkout -B [branch-name]`). Subject should be a small descriptive name what your branch is supposed to implement/fix. For example `drobert-settings_parsing` or `drobert-cgi_env_vars`.
3) After you're fixed/implemented what you wanted in your branch make a pull request. The pull request should have **EVERYTHING** that you changed in the description. Please add as much context as you can. 
4) The pull request will either get accepted (good job!) or will get denied. If it is denied there will be comments explaining what is expected to change before you try to PR again.

## some extra pointers:
1) Try to keep commits small. This makes it easier to see what is changed. And it allows you to more precisely go back to the version you want.
2) Have descriptive names for your commits. "Some fixes" doesn't tell anyone what has changed in that commit. It would be better to have a commit message like "Added support for multiple servers" or "Fixed issue #13".
3) Comment your code! But don't over do it, "This loops over all servers and finds the one with a given name" is good, "A list of servers" over the variable `server_list` is not helpful.
4) Try not to make your PR's too big. It makes it harder to review.