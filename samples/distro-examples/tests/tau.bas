Unit Tau
Import TauChild

Export expvar, foof, foop
export addRoom,calcRoomSize
export print_expvar, ta, build_ta,   cerr

expvar = "Tau's exported variable"

export const LIGHTGRAY  = rgb(200, 200, 200)
export const GRAY       = rgb(130, 130, 130)
export const DARKGRAY   = rgb(80, 80, 80), const YELLOW     = rgb(253, 249, 0)

func fooF(x)
	foof = "Tau's fooF("+x+") is here"
end

func mathF(x)
	mathF=x^2
end

sub fooP(p)
	? "Tau's fooP("+p+") is here"
end

sub TTchild
	TauChild.tc "Message from Tau"
end

sub print_expvar
	? expvar
	expvar = "message from tau"
end

sub build_ta
	ta = [1,2,3,4]
end

sub cerr
	throw 2
end

sub addRoom(the_thing,d)
end

func calcRoomSize(the_thing,d)
  calcRoomSize = 1
end

rem initialization
? "Tau::initilized"
?


