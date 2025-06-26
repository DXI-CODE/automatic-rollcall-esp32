document.getElementById("form-alumno").addEventListener("submit", function(e) {
    e.preventDefault();

    const nombre = document.getElementById("nombre").value.trim();
    const apPat = document.getElementById("apPat").value.trim();
    const apMat = document.getElementById("apMat").value.trim();
    const matricula = document.getElementById("matricula").value.trim();
    const semestre = document.getElementById("semestre").value.trim();
    const grupo = document.getElementById("grupo").value.trim();
    const carrera = document.getElementById("carrera").value.trim();
    const clase = document.getElementById("clase").value;
    const msgError = document.getElementById("msg-error");
    const btn = this.querySelector("button");

    // Quitar bordes de error anteriores
    ["nombre", "apPat", "apMat", "matricula", "semestre", "grupo", "carrera", "clase"].forEach(id => {
        document.getElementById(id).classList.remove('error-border');
    });

    // Validaciones generales
    if (!nombre || !apPat || !apMat || !matricula || !semestre || !grupo || !carrera || !clase) {
        msgError.textContent = "Por favor, completa todos los campos.";
        if (!nombre) document.getElementById("nombre").classList.add('error-border');
        if (!apPat) document.getElementById("apPat").classList.add('error-border');
        if (!apMat) document.getElementById("apMat").classList.add('error-border');
        if (!matricula) document.getElementById("matricula").classList.add('error-border');
        if (!semestre) document.getElementById("semestre").classList.add('error-border');
        if (!grupo) document.getElementById("grupo").classList.add('error-border');
        if (!carrera) document.getElementById("carrera").classList.add('error-border');
        if (!clase) document.getElementById("clase").classList.add('error-border');
        return;
    }

    const soloLetras = /^[A-Za-zÁÉÍÓÚáéíóúñÑ\s]+$/;
    if (!soloLetras.test(nombre) || !soloLetras.test(apPat) || !soloLetras.test(apMat)) {
        msgError.textContent = "Nombre y apellidos solo deben contener letras.";
        document.getElementById("nombre").classList.add('error-border');
        document.getElementById("apPat").classList.add('error-border');
        document.getElementById("apMat").classList.add('error-border');
        return;
    }

    if (!/^\d+$/.test(matricula)) {
        msgError.textContent = "La matrícula debe contener solo números.";
        document.getElementById("matricula").classList.add('error-border');
        return;
    }

    if (semestre < 1 || semestre > 12) {
        msgError.textContent = "El semestre debe ser entre 1 y 10.";
        document.getElementById("semestre").classList.add('error-border');
        return;
    }

    msgError.textContent = ""; // Limpiar mensaje de error

    const alumno = {
        nombre,
        apellido_paterno: apPat,
        apellido_materno: apMat,
        matricula,
        semestre,
        grupo,
        carrera,
        clase_id: parseInt(clase)
    };

    console.log("Alumno registrado:", alumno);

    // Bloquear botón para evitar múltiples envíos
    btn.disabled = true;

    // Simulación de envío
    setTimeout(() => {
        alert("Alumno guardado correctamente.");
        this.reset();
        btn.disabled = false;
    }, 500);

    // Si deseas enviar al ESP32 descomenta:
    /*
    fetch(`http://IP_DEL_ESP32/guardarAlumno`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(alumno)
    })
    .then(res => res.text())
    .then(data => {
        console.log(data);
        alert("Alumno guardado correctamente.");
        this.reset();
        btn.disabled = false;
    })
    .catch(err => {
        console.error(err);
        alert("Error al guardar.");
        btn.disabled = false;
    });
    */
});
