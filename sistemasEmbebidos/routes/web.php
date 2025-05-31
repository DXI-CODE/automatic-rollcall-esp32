<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\VistasController;

Route::get('/', function () {
    return view('welcome');
});

Route::get('/alumno', [VistasController::class, 'alumno']);
Route::get('/horario', [VistasController::class, 'horario']);



