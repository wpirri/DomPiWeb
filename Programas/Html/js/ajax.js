// Copyright � 2002-2010 Microchip Technology Inc.  All rights reserved.
// See Microchip TCP/IP Stack documentation for license information.


// Determines when a request is considered "timed out"
var timeOutMS = 30000; //ms (30s)
 
// Stores a queue of AJAX events to process
var ajaxList = new Array();

// Initiates a new AJAX command
//	url: the url to access
//	container: the document ID to fill, or a function to call with response XML (optional)
//	repeat: true to repeat this call indefinitely (optional)
//	data: an URL encoded string to be submitted as POST data (optional)
function newAJAXCommand(url, container, repeat, data)
{
	// Set up our object
	var newAjax = new Object();
	var theTimer = new Date();
	newAjax.url = url;
	newAjax.container = container;
	newAjax.repeat = repeat;
	newAjax.ajaxReq = null;
	
	// Create and send the request
	if(window.XMLHttpRequest) {
        newAjax.ajaxReq = new XMLHttpRequest();
        newAjax.ajaxReq.open((data==null)?"GET":"POST", newAjax.url, true);
        newAjax.ajaxReq.send(data);
    // If we're using IE6 style (maybe 5.5 compatible too)
    } else if(window.ActiveXObject) {
        newAjax.ajaxReq = new ActiveXObject("Microsoft.XMLHTTP");
        if(newAjax.ajaxReq) {
            newAjax.ajaxReq.open((data==null)?"GET":"POST", newAjax.url, true);
            newAjax.ajaxReq.send(data);
        }
    }
    
    newAjax.lastCalled = theTimer.getTime();
    
    // Store in our array
    ajaxList.push(newAjax);
}

// Loops over all pending AJAX events to determine if any action is required
function pollAJAX() {	
	var curAjax = new Object();
	var theTimer = new Date();
	var elapsed;

	try {
		// Read off the ajaxList objects one by one
		for(i = ajaxList.length; i > 0; i--)
		{
			curAjax = ajaxList.shift();
			if(!curAjax)
				continue;
			elapsed = theTimer.getTime() - curAjax.lastCalled;
					
			// If we suceeded
			if(curAjax.ajaxReq.readyState == 4 && curAjax.ajaxReq.status == 200) {
				// If it has a container, write the result
				if(typeof(curAjax.container) == 'function'){
					//curAjax.container(curAjax.ajaxReq.responseXML.documentElement);
					curAjax.container(curAjax.ajaxReq.responseText);
				} else if(typeof(curAjax.container) == 'string') {
					document.getElementById(curAjax.container).innerHTML = curAjax.ajaxReq.responseText;
				} // (otherwise do nothing for null values)
				
				curAjax.ajaxReq.abort();
				curAjax.ajaxReq = null;

				// If it's a repeatable request, then do so
				if(curAjax.repeat)
					newAJAXCommand(curAjax.url, curAjax.container, curAjax.repeat);
				continue;
			}
			
			// If we've waited over 1 second, then we timed out
			if(elapsed > timeOutMS) {
				// Invoke the user function with null input
				if(typeof(curAjax.container) == 'function'){
					curAjax.container(null);
				} else {
					// Alert the user
					alert("time-out");
				}

				curAjax.ajaxReq.abort();
				curAjax.ajaxReq = null;
				
				// If it's a repeatable request, then do so
				if(curAjax.repeat)
					newAJAXCommand(curAjax.url, curAjax.container, curAjax.repeat);
				continue;
			}
			
			// Otherwise, just keep waiting
			ajaxList.push(curAjax);
		}
	} catch (e) { }
	// Call ourselves again in 1s (original 100 ms)
	setTimeout("pollAJAX()",1000);
}
			
// Parses the xmlResponse returned by an XMLHTTPRequest object
//	xmlData: the xmlData returned
//  field: the field to search for
function getXMLValue(xmlData, field) {
	try {
		if(xmlData.getElementsByTagName(field)[0].firstChild.nodeValue)
			return xmlData.getElementsByTagName(field)[0].firstChild.nodeValue;
		else
			return null;
	} catch(err) { return null; }
}

// devuelve el valor del par�metro solicitado de la pagina que la llama
function GetUrlParam( name )
{
	name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");  
	var regexS = "[\\?&]"+name+"=([^&#]*)";  
	var regex = new RegExp( regexS );  
	var results = regex.exec( window.location.href );
	if( results == null )    return "";  
	else    return results[1];
}

function CheckErrorMessage()
{
	if((msg = GetUrlParam('error')))
	{
		msg = msg.replace(/\%20/g, " ").replace(/\%27/g, "");
		alert(msg);
	}	
}

//kick off the AJAX Updater
setTimeout("pollAJAX()",1000);
