<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <link rel="stylesheet" href="{{asset('/css/horario.css')}}">
    <title>Horarios</title>
</head>
<body>
    <div>
        <div class="head">
            <img src="{{asset('datos/unistmo.png')}}" class="unistmo">
            <img src="{{asset('datos/computacion.png')}}" class="computacion">
        </div>
        <div class="content">

           <a href="/views/welcome.blade.php" class="regresa-inicio">
                <img src="{{asset('datos/icon-home.webp')}}" alt="Inicio">
                Inicio
            </a>
            
            <div>
                <h1 class="description">CONTROL DE HORARIOS LABÓRATORIO DE ELECTRÓNICA E IA</h1>
            </div>

            <div class="search">
                <input type="text" placeholder="192.168.8.54" id="input">
                <button id="btn"><span>  <img src="{{asset('datos/search-icon.webp')}}" class="iconCal"> </img> </span></i></button>
            </div>
            <div class="print">
                
            </div>
        </div>
    </div>
    
    <div class="floating-menu" id="menu">
        <div class="row-1">
            <span class="btn-cerrar">X</span>
        </div>
        <div class="row-2">
        </div>
        <div class="row-3">
            <button id="edit">Editar</button>
            <button id="acep" style="display: none">Aceptar</button>
        </div>
    </div>
</body>
<script src="{{asset('/js/horario.js')}}"></script>
</html>
