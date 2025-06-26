<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="{{asset('/css/Alumnos.css')}}">
    <link href="https://unpkg.com/boxicons@2.1.4/css/boxicons.min.css" rel="stylesheet">

    <title>Registro de Alumnos</title>
    
</head>
<body>

    <h1>Registro de Alumnos</h1>

    <form id="form-alumno">
        <input type="text" id="nombre" placeholder="Nombre*">
        <input type="text" id="apPat" placeholder="Apellido Paterno*">
        <input type="text" id="apMat" placeholder="Apellido Materno*">
        <input type="text" id="matricula" placeholder="Matrícula*">
        <input type="number" id="semestre" placeholder="Semestre*" min="1" max="10">
        <input type="text" id="grupo" placeholder="Grupo*">
        <input type="text" id="carrera" placeholder="Carrera*">

        <select id="clase">
            <option value="">Selecciona una Clase</option>
            <option value="31">Electrónica</option>
            <option value="32">IA Avanzada</option>
        </select>

        <button type="submit">Guardar Alumno</button>

        <p class="error" id="msg-error"></p>
    </form>

</body>

<script src="{{asset('/js/Alumnos.js')}}"></script>
</html>
