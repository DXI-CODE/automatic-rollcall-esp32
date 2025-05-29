<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <link rel="stylesheet" href="{{asset('css/welcome.css')}}">
    <title>Inicio</title>
</head>
<body>
    <div>
        <div class="head">
            <img src="{{asset('datos/unistmo.png')}}" class="unistmo">
            <img src="{{asset('datos/computacion.png')}}" class="computacion">
        </div>
        <div class="content">
            <div>
                <h1 class="description">CONTROL DE HORARIOS LABÓRATORIO DE ELECTRÓNICA E IA</h1>
            </div>

            <div class="content-option">

                <div class="option" id="btn-h">
                    <span>  <img src="{{asset('datos/icon-cal.webp')}}" class="iconCal"></span>
                    <div class="line"></div>
                    <h3 class="datito">Configuración de Horarios</h3>
                </div>

                <div class="option" id="btn-a">
                    <span><span>  <img src="{{asset('datos/icon-alumno.webp')}}" class="iconAl"></span></span>
                    <div class="line"></div>
                    <h3 class="datito">Configuración de Alumnos</h3>
                </div>

            </div>

        </div>

    </div>
</body>
<script src="{{asset('/js/welcome.js')}}"></script>
</html>