function getCookie(name) {
    let value = "; " + document.cookie;
    let parts = value.split("; " + name + "=");
    if (parts.length == 2) return parts.pop().split(";").shift();
}

let cookieValue = getCookie( "json" );
if ( cookieValue ) {
    let decodedString = decodeURIComponent( cookieValue );
    let data = JSON.parse( decodedString );
    let date = data.genesis;
    let displayElement = document.getElementById("dateDisplay");
    displayElement.textContent = "Session creation date: " + date;
}


//console.log( date );
//console.log(document.cookie);