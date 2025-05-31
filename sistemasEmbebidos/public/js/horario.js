let aux;
let clase;
let ip;
let diaGlobal;
let numdia;

/*
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
                    if(data[key][otra].status == true){
                        div.innerHTML = `  <div class="class-row1">
                                            <div class="rectangulo"></div><div class="status-green"></div>
                                        </div>
                                        <div class="class-row2">
                                            <div class="class-colum1">
                                                <h3>${data[key][otra].nombre}</h3>
                                                <div class="grupo-content">
                                                    <p>Grupo:</p>
                                                    <p>${data[key][otra].grupo}</p>
                                                </div>
                                            </div>
                                            <div class="class-colum2">
                                                <i class='bx bx-trash'></i>
                                            </div>
                                        </div>
                                        `
                    }else{
                        div.innerHTML = `<div class="class-row1">
                                            <div class="rectangulo"></div><div class="status-red"></div>
                                        </div>
                                        <div class="class-row2">
                                            <div class="class-colum1">
                                                <h3>${data[key][otra].nombre}</h3>
                                                <div class="grupo-content">
                                                    <p>Grupo:</p>
                                                    <p>${data[key][otra].grupo}</p>
                                                </div>
                                            </div>
                                            <div class="class-colum2">
                                                <i class='bx bx-trash'></i>
                                            </div>
                                        </div>
                                        `
                    }
                    print.appendChild(div)
                    i++;
                }
            }
        }
        let add = document.createElement('div');
        add.classList.add('class-add');
        add.id = "add-class";
        add.innerHTML = `<i class='bx bx-plus'></i>` 
        print.appendChild(add);
    })
}*/

document.addEventListener("click", function(event) {
    let div = event.target.closest(".class");
    if (div) {
        document.querySelector("#menu").style.display = "block";
        id = div.id;
        mostrarInfo(id);
    }
    document.getElementById("edit").addEventListener("click", ()=>{
        editar(id);
    });
});

document.addEventListener("click", function(event){
    if(event.target.closest(".class-add")){
        document.querySelector("#menu").style.display = "block";
        let print = document.querySelector(".row-2");
        print.innerHTML = "";
        let div = document.createElement("div");
        div.innerHTML = `   <h3 id="nom-c">Nombre: <input type="text" id="nom-in" class="cambio"></h3>
                            <p id="gru-c">Grupo:  <input type="text" id="gru-in" class="cambio"></p>
                            <p id="hin-c">Hora de inicio: <input type="text" id="hin-in" class="cambio"> :
                                                            <input type="text" id="min-in" class="cambio"></p>
                            <p id="hfi-c">Hora de fin: <input type="text" id="hfi-in" class="cambio"> :
                                                        <input type="text" id="mfi-in" class="cambio"></p>
                            <p id="est-c">Estado: <input type="text" id="est-in" class="cambio"></p>`
        print.appendChild(div)
        let edit = document.getElementById("edit")
        edit.style.display = "none";
        let acep = document.getElementById("acep")
        acep.style.display = "block";
    }
})

document.getElementById("acep").addEventListener("click", ()=>{
    let nextIndex = Object.keys(aux[diaGlobal]).length;
    let nuevo = {
            nombre: document.getElementById("nom-in").value,
            grupo: document.getElementById("gru-in").value,
            h_inicio: document.getElementById("hin-in").value,
            m_inicio: document.getElementById("min-in").value,
            h_fin: document.getElementById("hfi-in").value,
            m_fin: document.getElementById("mfi-in").value,
            status: document.getElementById("est-in").value === true,
            id: (10*numdia)+(nextIndex)
        }
    aux[diaGlobal][nextIndex.toString()] = nuevo;
    console.log(aux)
    add();
});

function add(){
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
        document.querySelector("#menu").style.display = "none";
    })
    .catch(error => {
        console.error("Error sending data:", error);
    });
}

function mostrarInfo(id){
    let edit = document.getElementById("edit")
    edit.style.display = "block";
    let acep = document.getElementById("acep")
    acep.style.display = "none";
    let print = document.querySelector(".row-2");
    print.innerHTML = "";
    for(let key in aux){
        for(let otra in aux[key]){
            if(aux[key][otra].id == id){
                clase = aux[key][otra]
                let div = document.createElement("div");
                div.innerHTML = `   <h3 id="nom-c">${aux[key][otra].nombre}</h3>
                                    <p id="gru-c">Grupo: ${aux[key][otra].grupo}</p>
                                    <p id="hin-c">Hora de inicio: ${aux[key][otra].h_inicio}:${aux[key][otra].m_inicio}</p>
                                    <p id="hfi-c">Hora de fin: ${aux[key][otra].h_fin}:${aux[key][otra].m_fin}</p>
                                    <p id="est-c">Estado: ${aux[key][otra].status === true}</p>`
                                    
                print.appendChild(div)
            }
        }
    }
}

document.querySelector(".btn-cerrar").addEventListener("click", ()=>{
        document.querySelector("#menu").style.display = "none";
});

function editar(id){
    document.getElementById("nom-c").innerHTML = `<input type="text" id="nom-in" class="cambio" value="${clase.nombre}">`;
    document.getElementById("gru-c").innerHTML = `Grupo:  <input type="text" id="gru-in" class="cambio" value="${clase.grupo}">`;
    document.getElementById("hin-c").innerHTML = `Hora inicio:  <input type="text" id="hin-in" class="cambio" value="${clase.h_inicio}"> :
                                                                <input type="text" id="min-in" class="cambio" value="${clase.m_inicio}">`;
    document.getElementById("hfi-c").innerHTML = `Hora fin:  <input type="text" id="hfi-in" class="cambio" value="${clase.h_fin}"> :
                                                                <input type="text" id="mfi-in" class="cambio" value="${clase.m_fin}">`;
    document.getElementById("est-c").innerHTML = `Estado:  <input type="text" id="est-in" class="cambio" value="${clase.status}">`;
    let acep = document.getElementById("acep")
    acep.style.display = "block";
    let edit = document.getElementById("edit")
    edit.style.display = "none";
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
            edit.style.display = "block";
            acep.style.display = "none";
        })
        .catch(error => {
            console.error("Error sending data:", error);
        });
    })
}

document.querySelector(".regresa-inicio").addEventListener("click", ()=>{
    window.location.href = "/";
});


//Codigo para la placa
document.getElementById("btn").addEventListener("click", ()=>{
    let dia = new Date();
    ip = document.getElementById("input").value;
    switch(dia.getDay()){
        case 0:
            diaGlobal = "Don";
            pedir("Don");
            break;
        case 1:
            diaGlobal = "Lun";
            numdia = 1;
            pedir("Lun");
            break;
        case 2:
            diaGlobal = "Mar";
            numdia = 2;
            pedir("Mar");
            break;
        case 3:
            diaGlobal = "Mie";
            numdia = 3;
            pedir("Mie");
            break;
        case 4:
            diaGlobal = "Jue";
            numdia = 4;
            pedir("Jue");
            break;
        case 5:
            diaGlobal = "Vie";
            numdia = 5;
            pedir("Vie");
            break;
        case 6:
            diaGlobal = "Sab";
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
                    if(data[key][otra].status == true){
                        div.innerHTML = `  <div class="class-row1">
                                            <div class="rectangulo"></div><div class="status-green"></div>
                                        </div>
                                        <div class="class-row2">
                                            <div class="class-colum1">
                                                <h3>${data[key][otra].nombre}</h3>
                                                <div class="line-v"></div>
                                                <div class="grupo-content">
                                                    <p>Grupo:</p>
                                                    <p>${data[key][otra].grupo}</p>
                                                </div>
                                            </div>
                                            <div class="class-colum2">
                                                <i class='bx bx-trash'></i>
                                            </div>
                                        </div>
                                        `
                    }else{
                        div.innerHTML = `<div class="class-row1">
                                            <div class="rectangulo"></div><div class="status-red"></div>
                                        </div>
                                        <div class="class-row2">
                                            <div class="class-colum1">
                                                <h3>${data[key][otra].nombre}</h3>
                                                <div class="line-v"></div>
                                                <div class="grupo-content">
                                                    <p>Grupo:</p>
                                                    <p>${data[key][otra].grupo}</p>
                                                </div>
                                            </div>
                                            <div class="class-colum2">
                                                <i class='bx bx-trash'></i>
                                            </div>
                                        </div>
                                        `
                    }
                    print.appendChild(div)
                    i++;
                }
            }
        }
        let add = document.createElement('div');
        add.classList.add('class-add');
        add.id = "add-class";
        add.innerHTML = `<i class='bx bx-plus'></i>` 
        print.appendChild(add);
    })
}