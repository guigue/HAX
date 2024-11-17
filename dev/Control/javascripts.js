



//CODIGO TESTE

/* Java Script */
/* Socket Start Packet */

/*
 * Inicia o telescopio
 */

var Out;
var Error;
var Connected;
var Park;

//if (sky6RASCOMTele.IsHomed == 0 ){

//sky6RASCOMTele.Connect();
//sky6RASCOMTele.FindHome();
sky6RASCOMTele.Park();
//sky6RASCOMTele.ParkAndDoNotDisconnect();

Error = sky6RASCOMTele.LastSlewError;
Connected =  sky6RASCOMTele.IsConnected;
Park = sky6RASCOMTele.IsParked();



Out = "Connection= " + Connected + " | Error= " + Error + " | Parked= " + Park + " ";

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função start()

/* Java Script */
/* Socket Start Packet */

/*
 * Inicia o Telescopio
 */

var Out;

if (sky6RASCOMTele.IsParked != 0 ){
        sky6RASCOMTele.Unpark();
        Out = "Telescope Iniciated";
}

Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função connect()

/* Java Script */
/* Socket Start Packet */

/*
 * Testa e realiza a conexao com o telescopio
 */

var Out;

if (sky6RASCOMTele.IsConnected == 0 ){
        sky6RASCOMTele.Connect();
}

if (sky6RASCOMTele.IsConnected == 1 ){
        Out = "Connected";
}else{
	Out = "Not connected, an error ocurred while trying connecting. Unable to connect.";
}

Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função SetHome()

/* Java Script */
/* Socket Start Packet */

/*
 * Testa e determina o Home Position do telescopio
 */

var Out;

if (sky6RASCOMTele.LastSlewError == 0 ){
        sky6RASCOMTele.FindHome();
}

if (sky6RASCOMTele.IsHomed == 1 ){
        Out = "Is Homed";
}else{
        Out = sky6RASCOMTele.IsHomed;
}

Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função parkNotDisconnect()

/* Java Script */
/* Socket Start Packet */

/*
 * Faz o parking do telescopio e nao desconecta
 */

var Out;

if (sky6RASCOMTele.IsParked != 0 ){
	sky6RASCOMTele.ParkAndDoNotDisconnect();
	Out = "The Telescope is Parked";
}

Out;

/* Socket End Packet */

//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função park()

/* Java Script */
/* Socket Start Packet */

/*
 * Faz o parking do telescopio e disconecta ele
 */

var Out;

if (sky6RASCOMTele.IsParked != 0 ){
        sky6RASCOMTele.Park();
        Out = "Telescope is parked";
}

Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função stop()

/* Java Script */
/* Socket Start Packet */

/*
 * Desliga o tracking e faz o parking
 */

var Out;

sky6RASCOMTele.Abort();
sky6RASCOMTele.SetTracking(0,1,0,0);

if (sky6RASCOMTele.IsParked != 0 ){
        sky6RASCOMTele.Park();
        Out = "Telescope Disconnected and Parked";
}

Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função flip()

/* Java Script */
/* Socket Start Packet */

/*
 * Realiza a funcao de flip
 */

function main(){
	var Out = "";
	var err;
	sky6Utils.ComputeUniversalTime();
	var time = sky6Utils.dOut0();

	if(time == 16.0){
		if (sky6RASCOMTele.IsTracking == 0){
			sky6StarChart.LASTCOMERROR = 0;
			sky6StarChart.Find("@@1@@");
			err = sky6StarChart.LASTCOMERROR;
			if (err != 0){
				Out = Target + " not found.";
			}else{
				sky6ObjectInformation.Property(54);
				var targetRA = sky6ObjectInformation.ObjInfoPropOut;
				sky6ObjectInformation.Property(55);
				var targetDEC = sky6ObjectInformation.ObjInfoPropOut;

				sky6RASCOMTele.Asynchronous = true;
				sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, "@@1@@");
				Out = "Flip done!";
				return;
			}
		}else{
			Out = "Telescope is not tracking, first track any object.";
		}
	}else{
		Out = "The time is not 16h yet (UT)";
	}
}
main();
Out = Out;

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função point()

/* Java Script */
/* Socket Start Packet */

/*
 * Acha todas as informacoes do objeto encontrado
 * O uso dessa funcao nao trava a IDE do TheSkyX ao contrario da funcao pointAzAlt ou pointRaDec
 */

function wait(ms){
	var start = new Date().getTime();
	var end = start;
	while(end < start + ms){
		end - new Date().getTime();
	}
}

var Out;
var SlewComplete = 0;

sky6Web.SlewToObject("@@1@@");
while(SlewComplete != 1){
	wait(5000);
	SlewComplete = sky6Web.IsSlewComplete;
}

Out = "@@1@@ found and pointed to this object.";

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função monitoringRead()

/* Java Script */
/* Socket Start Packet */

/*
 * Pega as posicoes do que esta sendo observado no momento, entre eles:
 * Azimute, Elevacao, Ascensao Reta e Declinacao
 */

var Out;

sky6RASCOMTele.GetAzAlt();
var azimute = sky6RASCOMTele.dAz;
var altitude = sky6RASCOMTele.dAlt;
sky6RASCOMTele.GetRaDec();
var rightAscention = sky6RASCOMTele.dRa;
var declination = sky6RASCOMTele.dDec;

Out = "Azimute = " + String(azimute) + " | Altitude = " + String(altitude) + "\nRight Ascention = " + String(rightAscention) + "Declination = " + String(declination);

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função sky()

/* Java Script */
/* Socket Start Packet */

/*
 * Pega o azimute e altitude do telescopio, adiciona +5 ao azimute,
 * e faz o tracking sideral
 */

function wait(ms){
	var start = new Date().getTime();
	var end = start;
	while(end < start + ms){
		end = new Date().getTime();
	}
}

var Out = "";
var SlewComplete = 0;

sky6RASCOMTele.GetAzAlt();
var az = sky6RASCOMTele.dAz;
var alt = sky6RASCOMTele.dAlt;

var newAz = az + 5;

sky6RASCOMTele.SlewToAzAlt(newAz, alt, "");
while(SlewComplete != 1){
	wait(5000);
	SlewComplete = sky6Web.IsSlewComplete;
}
sky6RASCOMTele.SetTracking(1, 1, 0, 0);
// 1,1,0,0 = SiderealTracking

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função skyDip()

/* Java Script */
/* Socket Start Packet */

/*
 * Aborta o Sky, pega o azimute e ignora a elevacao, adiciona +10 na elevacao a cada 10 segundos,
 * e faz o slew comecando da elevacao = 10.
 */

function wait(ms){
        var start = new Date().getTime();
        var end = start;
        while(end < start + ms){
                end = new Date().getTime();
        }
}

var Out = "";
var SlewComplete = 0;

sky6RASCOMTele.Abort();
wait(10000);
sky6RASCOMTele.GetAzAlt();
var az = sky6RASCOMTele.dAz;
var alt = 10;

for (alt; alt < 91; alt +=10){
	sky6RASCOMTele.SlewToAzAlt(az, alt, "");
	while(sc != 1){
		wait(5000);
		SlewComplete = sky6Web.IsSlewComplete;
	}
	wait(10000); //Espera de 10 segundos
}

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função track(targetObject = @@1@@)

/* Java Script */
/* Socket Start Packet */

/*
 * Procura o objeto, faz o slew para um determinado objeto,
 * e depois faz o tracking deste objeto usando o tracking_rate
 */

function wait(ms){
        var start = new Date().getTime();
        var end = start;
        while(end < start + ms){
                end = new Date().getTime();
        }
}

var Out="";
var err;
sky6StarChart.LASTCOMERROR = 0;
sky6StarChart.Find("Sun");
err = sky6StarChart.LASTCOMERROR;
if (err != 0){
	Out = Target + " not found.";
}else{
	sky6ObjectInformation.Property(54);
	var targetRA = sky6ObjectInformation.ObjInfoPropOut;
        sky6ObjectInformation.Property(55);
        var targetDec = sky6ObjectInformation.ObjInfoPropOut;
        sky6ObjectInformation.Property(77);
        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;
        sky6ObjectInformation.Property(78);
        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;

	sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, "Sun");
	var slewComplete = sky6Web.IsSlewComplete;
	while(slewComplete != 1){
		wait(5000);
		slewComplete = sky6Web.IsSlewComplete;
	}
	sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);
}
Out = "Slew to @@1@@ completed and setted Right Ascension and Declination tracking rates.";

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função pointAzAlt(targetObject)

/* Java Script */
/* Socket Start Packet */

/*
 * Faz o slew para um determinado objeto, dados azimute e elevacao
 * O uso dessa funcao bloqueia a IDE do Sky ate o telescopio chegar ao destino
 */

function wait(ms){
        var start = new Date().getTime();
        var end = start;
        while(end < start + ms){
                end = new Date().getTime();
        }
}

var Out = "";
var SlewComplete = 0;
var err;

sky6StarChart.LASTCOMERROR = 0;
sky6StarChart.Find("@@1@@");
err = sky6StarChart.LASTCOMERROR;

if (err != 0){
	Out = Target + " not found.";
}else{
	sky6ObjectInformation.Property(58);
	var targetAz = sky6ObjectInformation.ObjInfoPropOut;
	sky6ObjectInformation.Property(59);
        var targetAlt = sky6ObjectInformation.ObjInfoPropOut;

	sky6RASCOMTele.Asynchronous = true;
	sky6RASCOMTele.SlewToAzAlt(targetAz, targetAlt, "@@1@@");

	while(SlewComplete != 1){
		wait(5000);
		SlewComplete = sky6Web.IsSlewComplete;
	}
	Out = "Slew to @@1@@ completed using Azimute and Altitude coordinates";
}

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================

//-------------------------------------------------------------
//Função pointRaDec(targetObject)

/* Java Script */
/* Socket Start Packet */

/*
 * Faz o slew para um determinado objeto, dados ascencao reta e declinacao
 * O uso dessa funcao bloqueia a IDE do Sky ate o telescopio chegar ao destino
 */

function wait(ms){
        var start = new Date().getTime();
        var end = start;
        while(end < start + ms){
                end = new Date().getTime();
        }
}

var Out = "";
var SlewComplete = 0;
var err;

sky6StarChart.LASTCOMERROR = 0;
sky6StarChart.Find("@@1@@");
err = sky6StarChart.LASTCOMERROR;

if (err != 0){
        Out = Target + " not found.";
}else{
        sky6ObjectInformation.Property(54);
        var targetRA = sky6ObjectInformation.ObjInfoPropOut;
        sky6ObjectInformation.Property(55);
        var targetDec = sky6ObjectInformation.ObjInfoPropOut;

        sky6RASCOMTele.Asynchronous = true;
        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, "@@1@@");

        while(SlewComplete != 1){
                wait(5000);
                SlewComplete = sky6Web.IsSlewComplete;
        }
        Out = "Slew to @@1@@ completed using Right Ascension and Declination coordinates";
}

/* Socket End Packet */
//-------------------------------------------------------------

//==========================================================================================
























