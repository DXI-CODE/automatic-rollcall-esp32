// Variables globales
let aux;
let ip;

// Redirigir a página de inicio
const btnInicio = document.getElementById("btn-a");
if (btnInicio) {
  btnInicio.addEventListener("click", () => {
    window.location.href = "/alumno";
  });
}

// Evento para el botón principal
const btnConsultar = document.getElementById("btn");
if (btnConsultar) {
  btnConsultar.addEventListener("click", () => {
    const dia = new Date().getDay();
    ip = document.getElementById("input").value.trim();
    if (!ip) {
      alert("Por favor, ingresa una IP válida.");
      return;
    }
    const dias = ["Don", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"];
    pedir(dias[dia]);
  });
}

// Obtener datos desde el ESP32
function pedir(day) {
  fetch(`http://${ip}/enviar`)
    .then((res) => res.json())
    .then((data) => {
      const print = document.querySelector(".print");
      print.innerHTML = "";
      aux = data;
      if (data[day]) {
        data[day].forEach((item) => {
          const div = document.createElement("div");
          div.classList.add("class");
          div.id = item.id;
          div.innerHTML = `
            <div class="class-row1">
              <div class="rectangulo"></div><div class="status"></div>
            </div>
            <h3>${item.nombre}</h3>
            <p>Grupo: ${item.grupo}</p>`;
          print.appendChild(div);
        });
      }
    })
    .catch((err) => console.error("Error obteniendo datos:", err));
}

// Mostrar menú de edición al hacer clic en un elemento
document.addEventListener("click", (event) => {
  if (event.target.matches(".class")) {
    const id = event.target.id;
    document.querySelector("#menu").style.display = "block";
    mostrarInfo(id);
    document.getElementById("edit").onclick = () => editar(id);
  }
});

// Cerrar menú
const btnCerrar = document.querySelector(".btn-cerrar");
if (btnCerrar) {
  btnCerrar.addEventListener("click", () => {
    document.querySelector("#menu").style.display = "none";
  });
}

// Mostrar información del ítem seleccionado
function mostrarInfo(id) {
  const print = document.querySelector(".row-2");
  print.innerHTML = "";
  for (let key in aux) {
    aux[key].forEach((item, i) => {
      if (item.id == id) {
        const div = document.createElement("div");
        div.id = i + 1;
        div.innerHTML = `
          <h3 id="nom-c">${item.nombre}</h3>
          <p id="gru-c">Grupo: ${item.grupo}</p>
          <p id="hin-c">Hora de inicio: ${item.h_inicio}:${item.m_inicio}</p>
          <p id="hfi-c">Hora de fin: ${item.h_fin}:${item.m_fin}</p>
          <p id="est-c">Estado: ${item.status}</p>`;
        print.appendChild(div);
      }
    });
  }
}

// Habilitar edición del ítem
function editar(id) {
  document.getElementById("nom-c").innerHTML = `<input type="text" id="nom-in" class="cambio">`;
  document.getElementById("gru-c").innerHTML = `Grupo: <input type="text" id="gru-in" class="cambio">`;
  document.getElementById("hin-c").innerHTML = `Hora inicio: <input type="text" id="hin-in" class="cambio"> : <input type="text" id="min-in" class="cambio">`;
  document.getElementById("hfi-c").innerHTML = `Hora fin: <input type="text" id="hfi-in" class="cambio"> : <input type="text" id="mfi-in" class="cambio">`;
  document.getElementById("est-c").innerHTML = `Estado: <input type="text" id="est-in" class="cambio">`;

  const btnAceptar = document.getElementById("acep");
  btnAceptar.style.display = "block";
  btnAceptar.onclick = () => guardarCambios(id);
}

// Guardar cambios editados
function guardarCambios(id) {
  const btnAceptar = document.getElementById("acep");
  btnAceptar.style.display = "none";

  const campos = {
    nombre: document.getElementById("nom-in").value.trim(),
    grupo: document.getElementById("gru-in").value.trim(),
    h_inicio: document.getElementById("hin-in").value.trim(),
    m_inicio: document.getElementById("min-in").value.trim(),
    h_fin: document.getElementById("hfi-in").value.trim(),
    m_fin: document.getElementById("mfi-in").value.trim(),
    status: document.getElementById("est-in").value.trim(),
  };

  for (let key in aux) {
    aux[key].forEach((item) => {
      if (item.id == id) {
        for (let campo in campos) {
          if (campos[campo] !== "") {
            item[campo] = campos[campo];
          }
        }
      }
    });
  }

  fetch(`http://${ip}/recibir`, {
    method: "POST",
    headers: { "Content-Type": "text/plain" },
    body: JSON.stringify(aux),
  })
    .then((res) => res.text())
    .then((result) => {
      console.log("ESP32 respondió:", result);
    })
    .catch((err) => {
      console.error("Error al enviar datos:", err);
    });
}
