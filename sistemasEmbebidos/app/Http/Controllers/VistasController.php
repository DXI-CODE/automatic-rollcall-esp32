<?php

namespace App\Http\Controllers;
use Illuminate\Http\Request;

class VistasController extends Controller
{
    function horario(){
        return view('horario');
    }

    function alumno(){
        return view('alumno');
    }
}
