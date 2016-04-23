function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject("Microsoft.XMLHTTP")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||"");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==""){C.open("POST",B,true);C.setRequestHeader("X-Requested-With","XMLHttpRequest");C.setRequestHeader("Content-type","application/x-www-form-urlencoded");C.setRequestHeader("Connection","close")}else{C.open("GET",B,true)}C.send(this.postBody)}};

function setValues(url)
{
	microAjax(url, function (res)
	{
		res.split(String.fromCharCode(10)).forEach(function(entry) {
			var fields = entry.split("|");
			try{
				if(fields[0] == "input")
				{
						document.getElementById(fields[1]).value = fields[2];
				}
				else if(fields[0] == "div")
				{
						document.getElementById(fields[1]).innerHTML  = fields[2];
				}
				else if(fields[0] == "chk")
				{
						document.getElementById(fields[1]).checked  = fields[2];
				}
				else if(fields[0] == "tab")
				{
						table = document.getElementById(fields[1]).getElementsByTagName("tbody")[0];
						table.innerHTML="";
						rows = Number(fields[3]);
						coloms = Number(fields[2]);
						for (i=0; i<rows; i++) {
							row=document.createElement("tr");
							for (j=0; j<coloms; j++)    {
								cell=document.createElement("td");
								cell.appendChild(document.createTextNode(fields[4+coloms*i+j]));
								row.appendChild(cell);
							}
							table.appendChild(row);
						}
				}
			}catch(err) {}
	  	});
		try{drawDayNightMap(mapImage);} catch(err) {}
	});
}
