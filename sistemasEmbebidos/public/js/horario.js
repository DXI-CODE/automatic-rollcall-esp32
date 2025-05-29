let aux;
let ip

document.getElementById("btn").addEventListener("click", ()=>{
    let dia = new Date();
    switch(dia.getDay()){
        case 0:
            pedir("Don");
            break;
        case 1:
            pedir("Lun");
            break;
        case 2:
            pedir("Mar");
            break;
        case 3:
            pedir("Mie");
            break;
        case 4:
            pedir("Jue");
            break;
        case 5:
            pedir("Vie");
            break;
        case 6:
            pedir("Sab");
            break;
    }
})


function pedir(day){
    fetch("/datos/archivo.json")
    .then(data => data.json())
    .then(data =>{
        let print = document.querySelector(".print");
        let i = 1;
        print.innerHTML = "";
        aux = data;
        for(let key in data){
            for(let otra in data[key]){
                if(day == key){
                    let div = document.createElement("div");
                    div.classList.add("class");
                    div.id = data[key][otra].id;
                    div.innerHTML = `  <div class="class-row1">
                                            <div class="rectangulo"></div><div class="status"></div>
                                        </div>
                                        <h3>${data[key][otra].nombre}</h3>
                                        <p>Grupo: ${data[key][otra].grupo}</p>`
                    print.appendChild(div)
                    i++;
                }
            }
        }
    })
}

document.addEventListener("click", function(event) {
  if (event.target.matches(".class")) {
    document.querySelector("#menu").style.display = "block";
    let id = event.target.id;
    mostrarInfo(id);
    document.getElementById("edit").addEventListener("click", ()=>{
        editar(id);
    });
  }
});

function mostrarInfo(id){
    let print = document.querySelector(".row-2");
    let i = 1;
    print.innerHTML = "";
    for(let key in aux){
        for(let otra in aux[key]){
            if(aux[key][otra].id == id){
                let div = document.createElement("div");
                div.id = i;
                div.innerHTML = `   <h3 id="nom-c">${aux[key][otra].nombre}</h3>
                                    <p id="gru-c">Grupo: ${aux[key][otra].grupo}</p>
                                    <p id="hin-c">Hora de inicio: ${aux[key][otra].h_inicio}:${aux[key][otra].m_inicio}</p>
                                    <p id="hfi-c">Hora de fin: ${aux[key][otra].h_fin}:${aux[key][otra].m_fin}</p>
                                    <p id="est-c">Estado: ${aux[key][otra].status}</p>`
                                    
                print.appendChild(div)
                i++;
            }
        }
    }
}

document.querySelector(".btn-cerrar").addEventListener("click", ()=>{
        document.querySelector("#menu").style.display = "none";
});

function editar(id){
    document.getElementById("nom-c").innerHTML = `<input type="text" id="nom-in" class="cambio">`;
    document.getElementById("gru-c").innerHTML = `Grupo:  <input type="text" id="gru-in" class="cambio">`;
    document.getElementById("hin-c").innerHTML = `Hora inicio:  <input type="text" id="hin-in" class="cambio"> :
                                                                <input type="text" id="min-in" class="cambio">`;
    document.getElementById("hfi-c").innerHTML = `Hora fin:  <input type="text" id="hfi-in" class="cambio"> :
                                                                <input type="text" id="mfi-in" class="cambio">`;
    document.getElementById("est-c").innerHTML = `Estado:  <input type="text" id="est-in" class="cambio">`;
    let acep = document.getElementById("acep")
    acep.style.display = "block";
    acep.addEventListener("click", ()=>{
        acep.style.display = "none";
        for(let key in aux){
            for(let otra in aux[key]){
                if(aux[key][otra].id == id){
                    if(document.getElementById("nom-in").value != ""){
                        aux[key][otra].nombre = document.getElementById("nom-in").value;
                    }
                    if(document.getElementById("gru-in").value != ""){
                        aux[key][otra].grupo = document.getElementById("gru-in").value;
                    }

                    if(document.getElementById("hin-in").value != ""){
                        aux[key][otra].h_inicio = document.getElementById("hin-in").value;
                    }

                    if(document.getElementById("min-in").value != ""){
                        aux[key][otra].m_inicio = document.getElementById("min-in").value;
                    }

                    if(document.getElementById("hfi-in").value != ""){
                        aux[key][otra].h_fin = document.getElementById("hfi-in").value;
                    }
                    
                    if(document.getElementById("mfi-in").value != ""){
                        aux[key][otra].m_fin = document.getElementById("mfi-in").value;
                    }
                    
                    if(document.getElementById("est-in").value != ""){
                        aux[key][otra].status = document.getElementById("est-in").value;
                    }
                }
            }
        }
        fetch(`http://${ip}/recibir`, {
            method: "POST",
            headers: {
            "Content-Type": "text/plain"
            },
            body: JSON.stringify(aux)
        })
        .then(response => response.text())
        .then(result => {
            console.log("ESP32 responded:", result);
        })
        .catch(error => {
            console.error("Error sending data:", error);
        });
    })
}

/*
//Codigo para la placa
document.getElementById("btn").addEventListener("click", ()=>{
    let dia = new Date();
    ip = document.getElementById("input").value;
    switch(dia.getDay()){
        case 0:
            pedir("Don");
            break;
        case 1:
            pedir("Lun");
            break;
        case 2:
            pedir("Mar");
            break;
        case 3:
            pedir("Mie");
            break;
        case 4:
            pedir("Jue");
            break;
        case 5:
            pedir("Vie");
            break;
        case 6:
            pedir("Sab");
            break;
    }
})

function pedir(day){
    fetch(`http://${ip}/enviar`)
    .then(data => data.json())
    .then(data =>{
        let print = document.querySelector(".print");
        let i = 1;
        print.innerHTML = "";
        aux = data;
        for(let key in data){
            for(let otra in data[key]){
                if(day == key){
                    let div = document.createElement("div");
                    div.classList.add("class");
                    div.id = data[key][otra].id;
                    div.innerHTML = `   
                                        <div class="class-row1">
                                            <div class="rectangulo"></div><div class="status"></div>
                                        </div>
                                        <h3>${data[key][otra].nombre}</h3>
                                        <p>Grupo: ${data[key][otra].grupo}</p>
                                        `
                    print.appendChild(div)
                    i++;
                }
            }
        }
    })
}*/