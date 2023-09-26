## Files
SHELL       = /bin/zsh

FNAMES      = main.cpp\
			  Server.cpp Router.cpp\
			  HttpMessage.cpp HttpResponse.cpp HttpRequest.cpp RequestFactory.cpp\
 			  ServerConfig.cpp ServerConfigCgiParser.cpp ServerConfigMimeParser.cpp\
 			  ServerConfigServerParser.cpp ServerConfigRouteParser.cpp ServerConfigChecker.cpp\
 			  ParserUtils.cpp ParsingException.cpp ServerUtils.cpp

SRCS        = $(addprefix $(SRCS_DIR)/,$(FNAMES))
OBJS        = $(addprefix $(OBJS_DIR)/,$(notdir $(FNAMES:.cpp=.o)))
DEPS        = $(addprefix $(DEPS_DIR)/,$(notdir $(FNAMES:.cpp=.d)))

INCLUDE_DIR = include
SRCS_DIR    = src
OBJS_DIR    = build
DEPS_DIR    = $(OBJS_DIR)

## Compilation
NAME        = webserv

CXX         = c++
CXXFLAGS    = -std=c++98 -D VERBOSE=true -Wall -Werror -Wextra -pedantic
	      #-g -fsanitize=address -fsanitize=leak

INCLUDES    = -I $(INCLUDE_DIR)
LIBS        =

## Other
RM          = rm -rf
ECHO        = echo -e


## Colors
BLACK       = \033[0;30m
RED         = \033[0;31m
GREEN       = \033[0;32m
ORANGE      = \033[0;33m
BLUE        = \033[0;34m
PURPLE      = \033[0;35m
CYAN        = \033[0;36m
GRAY        = \033[0;37m
WHITE       = \033[0;38m
RESET       = \033[0m


## Targets
all: $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	@$(ECHO) "$(GREEN)>>>>> Compiling $(RESET)$(notdir $<)$(GREEN) -> $(RESET)$(notdir $@)$(RESET)"
	@$(CXX) $(CXXFLAGS) -D DEBUG=1 -MMD -MP -c $(INCLUDES) $< -o $@

$(OBJS_DIR):
	@mkdir $(OBJS_DIR)
	@$(ECHO) "$(CYAN)Creating directory $(BLUE)'$(WHITE)$(OBJS_DIR)$(BLUE)'$(RESET)"

# regular targets
$(NAME): $(OBJS)
	@$(ECHO) "$(GREEN)>>>>> Linking <<<<<$(RESET)"
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(NAME)

clean:
	@$(ECHO) "$(GREEN)>>>>> Cleaning <<<<<$(RESET)"
	@$(RM) $(OBJS) $(DEPS)

fclean: clean
	@$(ECHO) "Applying full clean"
	@$(RM) $(OBJS_DIR) $(DEPS_DIR) $(NAME) $(NAME_FT_SPEED) $(NAME_STD_SPEED)

re: fclean all

vars:
	@$(ECHO) "$(GREEN)CXXFLAGS: $(WHITE)$(CXXFLAGS)$(RESET)"
	@$(ECHO) "$(GREEN)CXX:      $(WHITE)$(CXX)$(RESET)"
	@$(ECHO) "$(GREEN)FNAMES:   $(WHITE)$(FNAMES)$(RESET)"
	@$(ECHO) "$(GREEN)SRCS:     $(WHITE)$(SRCS)$(RESET)"
	@$(ECHO) "$(GREEN)OBJS:     $(WHITE)$(OBJS)$(RESET)"
	@$(ECHO) "$(GREEN)DEPS:     $(WHITE)$(DEPS)$(RESET)"

.PHONY: all clean fclean re init

-include $(DEPS)
