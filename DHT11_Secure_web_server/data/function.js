function logoutButton() 
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/logout", true);
    xhr.send();
    setTimeout(function () 
    { 
        window.open("/loggedout", "_self"); 
    }, 1000);
}