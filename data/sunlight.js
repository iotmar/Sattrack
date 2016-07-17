
var points = 80;

function initFootcalc(){
	with(Math){
	var lon=parseFloat(document.getElementById("lon").innerHTML)*Math.PI/180.0;
	var lat=parseFloat(document.getElementById("lat").innerHTML)*Math.PI/180.0;
	var alt=parseFloat(document.getElementById("alt").innerHTML);
	var off=parseFloat(document.getElementById("off").innerHTML)*PI/180.0;
	var r=(PI/2)-acos(cos(off)/((alt+6367.4445)/6367.4445))+off;
	p1c=sin(lon)*cos(r);p1s=sin(lat)*cos(lon)*cos(r);p1=cos(lat)*cos(lon)*sin(r);
	p2c=-cos(lon)*cos(r);p2s=sin(lat)*sin(lon)*cos(r);p2=cos(lat)*sin(lon)*sin(r);
	p3=sin(lat)*sin(r);p3s=-cos(lat)*cos(r);
	}
}
function footcalc(t){
	with(Math){
	var x=p1c*cos(t)+p1s*sin(t)+p1;
	var y=p2c*cos(t)+p2s*sin(t)+p2;
	var z=p3s*sin(t)+p3;
	return [atan2(y,x)*180.0/PI, atan2(z,sqrt(x*x+y*y))*180.0/PI];
	}
}
function footprint(ctx){
	initFootcalc();
	ctx.strokeStyle = 'white';
	var step=2*Math.PI/points;
	for (i=0;i<points;i++){
		pos=footcalc(step*i);
		if (pos[0]>0){pos[0] -= 360.0;}
		ctx.strokeRect(pixelX(-pos[0]),pixelY(pos[1]),1,1);
	};
}
// (C) 2011 g/christensen (gchristnsn@gmail.com)

// Map dimensions
var mapWidth = 750;
var mapHeight = 375;

function adjustParameters()
{
	pixelDegW = 360 / mapWidth;
	pixelDegH = 180 / mapHeight;
	edgeOffset = 0.0;
	centerDegW = (mapWidth / 2) * pixelDegW - edgeOffset;
	centerDegH = (mapHeight / 2) * pixelDegH;
}

function pixelX(deg)
{
	var offset = (deg < centerDegW)
		? (centerDegW - deg)
		: (360 - deg + centerDegW);

	return offset / pixelDegW;
}

function pixelY(deg)
{
	return (centerDegH - deg) / pixelDegH;
}

function pixelLambda(x)
{
	var deg = x * pixelDegW;
	return (deg < centerDegW)
		? (centerDegW - deg)
		: (360 - deg + centerDegW);
}

function pixelPhi(y, lambda)
{
	return centerDegH - y * pixelDegH;
}

function drawCircle(ctx, cx, cy, r, fill)
{
	ctx.beginPath();  
	ctx.arc(cx, cy, r, 0, Math.PI * 2, true);
	ctx.closePath();
	ctx.fillStyle = fill;
	ctx.fill();	
	ctx.stroke();
} 

function initmap()
{	
	adjustParameters();

	mapImage = new Image(); 
	mapImage.onload = function () {
		document.getElementById("hid").style.display = "initial";
		canvas = document.getElementById("dot");	
		canvas.width = mapWidth;
		canvas.height = mapHeight;
		map = document.getElementById("map");
		map.width = mapWidth;
		map.height = mapHeight;
		orbit = document.getElementById("orbit");	
		orbit.width = mapWidth;
		orbit.height = mapHeight;
		drawDayNightMap(mapImage);
		drawFootprint();
		drawOrbit();
		setInterval(function(){drawDayNightMap(mapImage);},60000);
		setInterval(drawFootprint,1000);
		};
	mapImage.src = "/map.jpg";
}

function drawFootprint(){

	var ctx = canvas.getContext("2d");
	
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	ctx.fillStyle = "rgba(0, 0, 0, 0.5)";
	var lon = - parseFloat(document.getElementById("lon").innerHTML);
	if (lon<0){lon += 360.0;}
	var lat = parseFloat(document.getElementById("lat").innerHTML);
	ctx.strokeStyle = 'black';
	drawCircle(ctx, pixelX(lon), pixelY(lat), 5, "#FF0000");
	footprint(ctx);
}

function drawOrbit(){

	var ctx = orbit.getContext("2d");
	
	ctx.clearRect(0, 0, orbit.width, orbit.height);
	ctx.fillStyle = "rgba(0, 0, 0, 0.5)";
	ctx.strokeStyle = 'yellow';
	for (i=0;i<orbit_size;i++){
		var lon = - orbitLon[i];
		if (lon<0){lon += 360.0;}
		ctx.strokeRect(pixelX(lon),pixelY(orbitLat[i]),1,1);
	};
}

function drawDayNightMap(mapImage)
{
	var ctx = map.getContext("2d");	

	ctx.drawImage(mapImage, 0, 0);
	performCalculations(time);

	var northSun = DECsun >= 0;
	var startFrom = northSun? 0: (mapHeight - 1);
	var pstop = function (y) { return northSun? (y < mapHeight): (y >= 0); };
	var inc = northSun? 1: -1;
	
	ctx.fillStyle = "rgba(0, 0, 0, 0.5)";

	for (var x = 0; x < mapWidth; ++x)
		for (var y = startFrom; pstop(y); y += inc)
		{			
			var lambda = pixelLambda(x);
			var phi = pixelPhi(y) + 0.5 * (northSun? -1: 1);

			var centralAngle = sind(phi) * sind(DECsun) 
							 + cosd(phi) * cosd(DECsun) * cosd(GHAsun - lambda);
			centralAngle = Math.acos(centralAngle);
			 
			if (centralAngle > Math.PI / 2)
			{                                              
				var rectTop = northSun? y: 0;
				var rectHeight = northSun? mapHeight - rectTop: y + 1;
                
				ctx.fillRect(x, rectTop, 1, rectHeight);                                             				
				break;
			}    
		}
	ctx.strokeStyle = 'black';
	var lon = - parseFloat(document.getElementById("clon").innerHTML);
	var lat = parseFloat(document.getElementById("clat").innerHTML);
	if (lon<0){lon += 360.0;}
	drawCircle(ctx, pixelX(lon), pixelY(lat), 3, "#00ff00");        
}		

// Source: Henning Umland, http://www.celnav.de/longterm.htm
var dtr=Math.PI/180;function sind(x){return Math.sin(dtr*x)}function cosd(x){return Math.cos(dtr*x)}function tand(x){return Math.tan(dtr*x)}function trunc(x){return 360*(x/360-Math.floor(x/360))}function performCalculations(date){year=date.getUTCFullYear();month=date.getUTCMonth()+1;day=date.getUTCDate();hour=date.getUTCHours();minute=date.getUTCMinutes();second=date.getUTCSeconds();deltaT=0;dayfraction=(hour+minute/60+second/3600)/24;TimeMeasures();Nutation();Aries();Sun()}function TimeMeasures(){with(Math){if(month<=2){year-=1;month+=12}var A=floor(year/100);var B=2-A+floor(A/4);JD0h=floor(365.25*(year+4716))+floor(30.6001*(month+1))+day+B-1524.5;JD=JD0h+dayfraction;T=(JD-2451545)/36525;T2=T*T;T3=T*T2;T4=T*T3;T5=T*T4;JDE=JD+deltaT/86400;TE=(JDE-2451545)/36525;TE2=TE*TE;TE3=TE*TE2;TE4=TE*TE3;TE5=TE*TE4;Tau=0.1*TE;Tau2=Tau*Tau;Tau3=Tau*Tau2;Tau4=Tau*Tau3;Tau5=Tau*Tau4}}function Nutation(){var Mm=134.962981389+198.867398056*TE+trunc(477000*TE)+0.008697222222*TE2+TE3/56250;var M=357.527723333+359.05034*TE+trunc(35640*TE)-0.0001602777778*TE2-TE3/300000;var F=93.271910277+82.017538055*TE+trunc(483120*TE)-0.0036825*TE2+TE3/327272.7273;var D=297.850363055+307.11148*TE+trunc(444960*TE)-0.001914166667*TE2+TE3/189473.6842;var omega=125.044522222-134.136260833*TE-trunc(1800*TE)+0.002070833333*TE2+TE3/450000;var nut=new Array(106);nut[0]=" 0 0 0 0 1-171996-174.2 92025 8.9 ";nut[1]=" 0 0 2-2 2 -13187  -1.6  5736-3.1 ";nut[2]=" 0 0 2 0 2  -2274  -0.2   977-0.5 ";nut[3]=" 0 0 0 0 2   2062   0.2  -895 0.5 ";nut[4]=" 0-1 0 0 0  -1426   3.4    54-0.1 ";nut[5]=" 1 0 0 0 0    712   0.1    -7 0.0 ";nut[6]=" 0 1 2-2 2   -517   1.2   224-0.6 ";nut[7]=" 0 0 2 0 1   -386  -0.4   200 0.0 ";nut[8]=" 1 0 2 0 2   -301   0.0   129-0.1 ";nut[9]=" 0-1 2-2 2    217  -0.5   -95 0.3 ";nut[10]="-1 0 0 2 0    158   0.0    -1 0.0 ";nut[11]=" 0 0 2-2 1    129   0.1   -70 0.0 ";nut[12]="-1 0 2 0 2    123   0.0   -53 0.0 ";nut[13]=" 1 0 0 0 1     63   0.1   -33 0.0 ";nut[14]=" 0 0 0 2 0     63   0.0    -2 0.0 ";nut[15]="-1 0 2 2 2    -59   0.0    26 0.0 ";nut[16]="-1 0 0 0 1    -58  -0.1    32 0.0 ";nut[17]=" 1 0 2 0 1    -51   0.0    27 0.0 ";nut[18]="-2 0 0 2 0    -48   0.0     1 0.0 ";nut[19]="-2 0 2 0 1     46   0.0   -24 0.0 ";nut[20]=" 0 0 2 2 2    -38   0.0    16 0.0 ";nut[21]=" 2 0 2 0 2    -31   0.0    13 0.0 ";nut[22]=" 2 0 0 0 0     29   0.0    -1 0.0 ";nut[23]=" 1 0 2-2 2     29   0.0   -12 0.0 ";nut[24]=" 0 0 2 0 0     26   0.0    -1 0.0 ";nut[25]=" 0 0 2-2 0    -22   0.0     0 0.0 ";nut[26]="-1 0 2 0 1     21   0.0   -10 0.0 ";nut[27]=" 0 2 0 0 0     17  -0.1     0 0.0 ";nut[28]=" 0 2 2-2 2    -16   0.1     7 0.0 ";nut[29]="-1 0 0 2 1     16   0.0    -8 0.0 ";nut[30]=" 0 1 0 0 1    -15   0.0     9 0.0 ";nut[31]=" 1 0 0-2 1    -13   0.0     7 0.0 ";nut[32]=" 0-1 0 0 1    -12   0.0     6 0.0 ";nut[33]=" 2 0-2 0 0     11   0.0     0 0.0 ";nut[34]="-1 0 2 2 1    -10   0.0     5 0.0 ";nut[35]=" 1 0 2 2 2     -8   0.0     3 0.0 ";nut[36]=" 0-1 2 0 2     -7   0.0     3 0.0 ";nut[37]=" 0 0 2 2 1     -7   0.0     3 0.0 ";nut[38]=" 1 1 0-2 0     -7   0.0     0 0.0 ";nut[39]=" 0 1 2 0 2      7   0.0    -3 0.0 ";nut[40]="-2 0 0 2 1     -6   0.0     3 0.0 ";nut[41]=" 0 0 0 2 1     -6   0.0     3 0.0 ";nut[42]=" 2 0 2-2 2      6   0.0    -3 0.0 ";nut[43]=" 1 0 0 2 0      6   0.0     0 0.0 ";nut[44]=" 1 0 2-2 1      6   0.0    -3 0.0 ";nut[45]=" 0 0 0-2 1     -5   0.0     3 0.0 ";nut[46]=" 0-1 2-2 1     -5   0.0     3 0.0 ";nut[47]=" 2 0 2 0 1     -5   0.0     3 0.0 ";nut[48]=" 1-1 0 0 0      5   0.0     0 0.0 ";nut[49]=" 1 0 0-1 0     -4   0.0     0 0.0 ";nut[50]=" 0 0 0 1 0     -4   0.0     0 0.0 ";nut[51]=" 0 1 0-2 0     -4   0.0     0 0.0 ";nut[52]=" 1 0-2 0 0      4   0.0     0 0.0 ";nut[53]=" 2 0 0-2 1      4   0.0    -2 0.0 ";nut[54]=" 0 1 2-2 1      4   0.0    -2 0.0 ";nut[55]=" 1 1 0 0 0     -3   0.0     0 0.0 ";nut[56]=" 1-1 0-1 0     -3   0.0     0 0.0 ";nut[57]="-1-1 2 2 2     -3   0.0     1 0.0 ";nut[58]=" 0-1 2 2 2     -3   0.0     1 0.0 ";nut[59]=" 1-1 2 0 2     -3   0.0     1 0.0 ";nut[60]=" 3 0 2 0 2     -3   0.0     1 0.0 ";nut[61]="-2 0 2 0 2     -3   0.0     1 0.0 ";nut[62]=" 1 0 2 0 0      3   0.0     0 0.0 ";nut[63]="-1 0 2 4 2     -2   0.0     1 0.0 ";nut[64]=" 1 0 0 0 2     -2   0.0     1 0.0 ";nut[65]="-1 0 2-2 1     -2   0.0     1 0.0 ";nut[66]=" 0-2 2-2 1     -2   0.0     1 0.0 ";nut[67]="-2 0 0 0 1     -2   0.0     1 0.0 ";nut[68]=" 2 0 0 0 1      2   0.0    -1 0.0 ";nut[69]=" 3 0 0 0 0      2   0.0     0 0.0 ";nut[70]=" 1 1 2 0 2      2   0.0    -1 0.0 ";nut[71]=" 0 0 2 1 2      2   0.0    -1 0.0 ";nut[72]=" 1 0 0 2 1     -1   0.0     0 0.0 ";nut[73]=" 1 0 2 2 1     -1   0.0     1 0.0 ";nut[74]=" 1 1 0-2 1     -1   0.0     0 0.0 ";nut[75]=" 0 1 0 2 0     -1   0.0     0 0.0 ";nut[76]=" 0 1 2-2 0     -1   0.0     0 0.0 ";nut[77]=" 0 1-2 2 0     -1   0.0     0 0.0 ";nut[78]=" 1 0-2 2 0     -1   0.0     0 0.0 ";nut[79]=" 1 0-2-2 0     -1   0.0     0 0.0 ";nut[80]=" 1 0 2-2 0     -1   0.0     0 0.0 ";nut[81]=" 1 0 0-4 0     -1   0.0     0 0.0 ";nut[82]=" 2 0 0-4 0     -1   0.0     0 0.0 ";nut[83]=" 0 0 2 4 2     -1   0.0     0 0.0 ";nut[84]=" 0 0 2-1 2     -1   0.0     0 0.0 ";nut[85]="-2 0 2 4 2     -1   0.0     1 0.0 ";nut[86]=" 2 0 2 2 2     -1   0.0     0 0.0 ";nut[87]=" 0-1 2 0 1     -1   0.0     0 0.0 ";nut[88]=" 0 0-2 0 1     -1   0.0     0 0.0 ";nut[89]=" 0 0 4-2 2      1   0.0     0 0.0 ";nut[90]=" 0 1 0 0 2      1   0.0     0 0.0 ";nut[91]=" 1 1 2-2 2      1   0.0    -1 0.0 ";nut[92]=" 3 0 2-2 2      1   0.0     0 0.0 ";nut[93]="-2 0 2 2 2      1   0.0    -1 0.0 ";nut[94]="-1 0 0 0 2      1   0.0    -1 0.0 ";nut[95]=" 0 0-2 2 1      1   0.0     0 0.0 ";nut[96]=" 0 1 2 0 1      1   0.0     0 0.0 ";nut[97]="-1 0 4 0 2      1   0.0     0 0.0 ";nut[98]=" 2 1 0-2 0      1   0.0     0 0.0 ";nut[99]=" 2 0 0 2 0      1   0.0     0 0.0 ";nut[100]=" 2 0 2-2 1      1   0.0    -1 0.0 ";nut[101]=" 2 0-2 0 1      1   0.0     0 0.0 ";nut[102]=" 1-1 0-2 0      1   0.0     0 0.0 ";nut[103]="-1 0 0 1 1      1   0.0     0 0.0 ";nut[104]="-1-1 0 2 1      1   0.0     0 0.0 ";nut[105]=" 0 1 0 1 0      1   0.0     0 0.0 ";var fMm,fM,fF,fD,f_omega,dp=0,de=0;for(x=0;x<105;x++){fMm=eval(nut[x].substring(0,2));fM=eval(nut[x].substring(2,4));fF=eval(nut[x].substring(4,6));fD=eval(nut[x].substring(6,8));f_omega=eval(nut[x].substring(8,10));dp+=(eval(nut[x].substring(10,17))+TE*eval(nut[x].substring(17,23)))*sind(fD*D+fM*M+fMm*Mm+fF*F+f_omega*omega);de+=(eval(nut[x].substring(23,29))+TE*eval(nut[x].substring(29,33)))*cosd(fD*D+fM*M+fMm*Mm+fF*F+f_omega*omega)}delta_psi=dp/36000000;delta_eps=de/36000000;eps0=(84381.448-46.815*TE-0.00059*TE2+0.001813*TE3)/3600;eps=eps0+delta_eps}function Aries(){var GHAAmean=trunc(280.46061837+360.98564736629*(JD-2451545)+0.000387933*T2-T3/38710000);GHAAtrue=trunc(GHAAmean+delta_psi*cosd(eps))}function Sun(){with(Math){var L0=175347046;L0+=3341656*cos(4.6692568+6283.0758500*Tau);L0+=34894*cos(4.62610+12566.15170*Tau);L0+=3497*cos(2.7441+5753.3849*Tau);L0+=3418*cos(2.8289+3.5231*Tau);L0+=3136*cos(3.6277+77713.7715*Tau);L0+=2676*cos(4.4181+7860.4194*Tau);L0+=2343*cos(6.1352+3930.2097*Tau);L0+=1324*cos(0.7425+11506.7698*Tau);L0+=1273*cos(2.0371+529.6910*Tau);L0+=1199*cos(1.1096+1577.3435*Tau);L0+=990*cos(5.233+5884.927*Tau);L0+=902*cos(2.045+26.298*Tau);L0+=857*cos(3.508+398.149*Tau);L0+=780*cos(1.179+5223.694*Tau);L0+=753*cos(2.533+5507.553*Tau);L0+=505*cos(4.583+18849.228*Tau);L0+=492*cos(4.205+775.523*Tau);L0+=357*cos(2.920+0.067*Tau);L0+=317*cos(5.849+11790.629*Tau);L0+=284*cos(1.899+796.298*Tau);L0+=271*cos(0.315+10977.079*Tau);L0+=243*cos(0.345+5486.778*Tau);L0+=206*cos(4.806+2544.314*Tau);L0+=205*cos(1.869+5573.143*Tau);L0+=202*cos(2.458+6069.777*Tau);L0+=156*cos(0.833+213.299*Tau);L0+=132*cos(3.411+2942.463*Tau);L0+=126*cos(1.083+20.775*Tau);L0+=115*cos(0.645+0.980*Tau);L0+=103*cos(0.636+4694.003*Tau);L0+=102*cos(0.976+15720.839*Tau);L0+=102*cos(4.267+7.114*Tau);L0+=99*cos(6.21+2146.17*Tau);L0+=98*cos(0.68+155.42*Tau);L0+=86*cos(5.98+161000.69*Tau);L0+=85*cos(1.30+6275.96*Tau);L0+=85*cos(3.67+71430.70*Tau);L0+=80*cos(1.81+17260.15*Tau);L0+=79*cos(3.04+12036.46*Tau);L0+=75*cos(1.76+5088.63*Tau);L0+=74*cos(3.50+3154.69*Tau);L0+=74*cos(4.68+801.82*Tau);L0+=70*cos(0.83+9437.76*Tau);L0+=62*cos(3.98+8827.39*Tau);L0+=61*cos(1.82+7084.90*Tau);L0+=57*cos(2.78+6286.60*Tau);L0+=56*cos(4.39+14143.50*Tau);L0+=56*cos(3.47+6279.55*Tau);L0+=52*cos(0.19+12139.55*Tau);L0+=52*cos(1.33+1748.02*Tau);L0+=51*cos(0.28+5856.48*Tau);L0+=49*cos(0.49+1194.45*Tau);L0+=41*cos(5.37+8429.24*Tau);L0+=41*cos(2.40+19651.05*Tau);L0+=39*cos(6.17+10447.39*Tau);L0+=37*cos(6.04+10213.29*Tau);L0+=37*cos(2.57+1059.38*Tau);L0+=36*cos(1.71+2352.87*Tau);L0+=36*cos(1.78+6812.77*Tau);L0+=33*cos(0.59+17789.85*Tau);L0+=30*cos(0.44+83996.85*Tau);L0+=30*cos(2.74+1349.87*Tau);L0+=25*cos(3.16+4690.48*Tau);var L1=628331966747;L1+=206059*cos(2.678235+6283.075850*Tau);L1+=4303*cos(2.6351+12566.1517*Tau);L1+=425*cos(1.590+3.523*Tau);L1+=119*cos(5.796+26.298*Tau);L1+=109*cos(2.966+1577.344*Tau);L1+=93*cos(2.59+18849.23*Tau);L1+=72*cos(1.14+529.69*Tau);L1+=68*cos(1.87+398.15*Tau);L1+=67*cos(4.41+5507.55*Tau);L1+=59*cos(2.89+5223.69*Tau);L1+=56*cos(2.17+155.42*Tau);L1+=45*cos(0.40+796.30*Tau);L1+=36*cos(0.47+775.52*Tau);L1+=29*cos(2.65+7.11*Tau);L1+=21*cos(5.34+0.98*Tau);L1+=19*cos(1.85+5486.78*Tau);L1+=19*cos(4.97+213.30*Tau);L1+=17*cos(2.99+6275.96*Tau);L1+=16*cos(0.03+2544.31*Tau);L1+=16*cos(1.43+2146.17*Tau);L1+=15*cos(1.21+10977.08*Tau);L1+=12*cos(2.83+1748.02*Tau);L1+=12*cos(3.26+5088.63*Tau);L1+=12*cos(5.27+1194.45*Tau);L1+=12*cos(2.08+4694.00*Tau);L1+=11*cos(0.77+553.57*Tau);L1+=10*cos(1.30+6286.60*Tau);L1+=10*cos(4.24+1349.87*Tau);L1+=9*cos(2.70+242.73*Tau);L1+=9*cos(5.64+951.72*Tau);L1+=8*cos(5.30+2352.87*Tau);L1+=6*cos(2.65+9437.76*Tau);L1+=6*cos(4.67+4690.48*Tau);var L2=52919;L2+=8720*cos(1.0721+6283.0758*Tau);L2+=309*cos(0.867+12566.152*Tau);L2+=27*cos(0.05+3.52*Tau);L2+=16*cos(5.19+26.30*Tau);L2+=16*cos(3.68+155.42*Tau);L2+=10*cos(0.76+18849.23*Tau);L2+=9*cos(2.06+77713.77*Tau);L2+=7*cos(0.83+775.52*Tau);L2+=5*cos(4.66+1577.34*Tau);L2+=4*cos(1.03+7.11*Tau);L2+=4*cos(3.44+5573.14*Tau);L2+=3*cos(5.14+796.30*Tau);L2+=3*cos(6.05+5507.55*Tau);L2+=3*cos(1.19+242.73*Tau);L2+=3*cos(6.12+529.69*Tau);L2+=3*cos(0.31+398.15*Tau);L2+=3*cos(2.28+553.57*Tau);L2+=2*cos(4.38+5223.69*Tau);L2+=2*cos(3.75+0.98*Tau);var L3=289*cos(5.844+6283.076*Tau);L3+=35;L3+=17*cos(5.49+12566.15*Tau);L3+=3*cos(5.20+155.42*Tau);L3+=1*cos(4.72+3.52*Tau);L3+=1*cos(5.30+18849.23*Tau);L3+=1*cos(5.97+242.73*Tau);var L4=114*cos(3.142);L4+=8*cos(4.13+6283.08*Tau);L4+=1*cos(3.84+12566.15*Tau);var L5=1*cos(3.14);var Lsun_mean=trunc(280.4664567+360007.6982779*Tau+0.03032028*Tau2+Tau3/49931-Tau4/15299-Tau5/1988000);var Lhelioc=trunc((L0+L1*Tau+L2*Tau2+L3*Tau3+L4*Tau4+L5*Tau5)/1e8/dtr);Lsun_true=trunc(Lhelioc+180-0.000025);var B0=280*cos(3.199+84334.662*Tau);B0+=102*cos(5.422+5507.553*Tau);B0+=80*cos(3.88+5223.69*Tau);B0+=44*cos(3.70+2352.87*Tau);B0+=32*cos(4.00+1577.34*Tau);var B1=9*cos(3.90+5507.55*Tau);B1+=6*cos(1.73+5223.69*Tau);var B=(B0+B1*Tau)/1e8/dtr;var beta=trunc(-B);Lsun_prime=trunc(Lhelioc+180-1.397*TE-0.00031*TE2);beta=beta+0.000011*(cosd(Lsun_prime)-sind(Lsun_prime));var R0=100013989;R0+=1670700*cos(3.0984635+6283.0758500*Tau);R0+=13956*cos(3.05525+12566.15170*Tau);R0+=3084*cos(5.1985+77713.7715*Tau);R0+=1628*cos(1.1739+5753.3849*Tau);R0+=1576*cos(2.8469+7860.4194*Tau);R0+=925*cos(5.453+11506.770*Tau);R0+=542*cos(4.564+3930.210*Tau);R0+=472*cos(3.661+5884.927*Tau);R0+=346*cos(0.964+5507.553*Tau);R0+=329*cos(5.900+5223.694*Tau);R0+=307*cos(0.299+5573.143*Tau);R0+=243*cos(4.273+11790.629*Tau);R0+=212*cos(5.847+1577.344*Tau);R0+=186*cos(5.022+10977.079*Tau);R0+=175*cos(3.012+18849.228*Tau);R0+=110*cos(5.055+5486.778*Tau);R0+=98*cos(0.89+6069.78*Tau);R0+=86*cos(5.69+15720.84*Tau);R0+=86*cos(1.27+161000.69*Tau);R0+=65*cos(0.27+17260.15*Tau);R0+=63*cos(0.92+529.69*Tau);R0+=57*cos(2.01+83996.85*Tau);R0+=56*cos(5.24+71430.70*Tau);R0+=49*cos(3.25+2544.31*Tau);R0+=47*cos(2.58+775.52*Tau);R0+=45*cos(5.54+9437.76*Tau);R0+=43*cos(6.01+6275.96*Tau);R0+=39*cos(5.36+4694.00*Tau);R0+=38*cos(2.39+8827.39*Tau);R0+=37*cos(0.83+19651.05*Tau);R0+=37*cos(4.90+12139.55*Tau);R0+=36*cos(1.67+12036.46*Tau);R0+=35*cos(1.84+2942.46*Tau);R0+=33*cos(0.24+7084.90*Tau);R0+=32*cos(0.18+5088.63*Tau);R0+=32*cos(1.78+398.15*Tau);R0+=28*cos(1.21+6286.60*Tau);R0+=28*cos(1.90+6279.55*Tau);R0+=26*cos(4.59+10447.39*Tau);var R1=103019*cos(1.107490+6283.075850*Tau);R1+=1721*cos(1.0644+12566.1517*Tau);R1+=702*cos(3.142);R1+=32*cos(1.02+18849.23*Tau);R1+=31*cos(2.84+5507.55*Tau);R1+=25*cos(1.32+5223.69*Tau);R1+=18*cos(1.42+1577.34*Tau);R1+=10*cos(5.91+10977.08*Tau);R1+=9*cos(1.42+6275.96*Tau);R1+=9*cos(0.27+5486.78*Tau);var R2=4359*cos(5.7846+6283.0758*Tau);R2+=124*cos(5.579+12566.152*Tau);R2+=12*cos(3.14);R2+=9*cos(3.63+77713.77*Tau);R2+=6*cos(1.87+5573.14*Tau);R2+=3*cos(5.47+18849.23*Tau);var R3=145*cos(4.273+6283.076*Tau);R3+=7*cos(3.92+12566.15*Tau);var R4=4*cos(2.56+6283.08*Tau);R=(R0+R1*Tau+R2*Tau2+R3*Tau3+R4*Tau4)/1e8;lambda=trunc(Lsun_true+delta_psi-0.005691611/R);RAsun=trunc(atan2((sind(lambda)*cosd(eps)-tand(beta)*sind(eps)),cosd(lambda))/dtr);SHAsun=360-RAsun;DECsun=asin(sind(beta)*cosd(eps)+cosd(beta)*sind(eps)*sind(lambda))/dtr;Dsun=DECsun;GHAsun=trunc(GHAAtrue-RAsun);SDsun=959.63/R;HPsun=8.794/R;EOT=4*GHAsun+720-1440*dayfraction;if(EOT>20)EOT-=1440;if(EOT<-20)EOT+=1440}}