/**
\file

\brief Funzioni fondamentali nell'utilizzo della classe

Il file contiene le funzioni `begin`, `messaggio`, `errore` e `controllaLed`

\date 5 - 7 luglio 2017

*/

#include "Debug.hpp"
#include "Debug_string.hpp"

//cfr. file debug_impostazioni.h per il senso di questa condizione
#ifdef DEBUG_ABILITA

//crea un'istanza della classe
Debug debug(Serial);


/**
Qesta funzione deve essere chiamata una volta all'inizio del programma.
Prima di essa si possono chiamare `usaSerial()` e `usaLed()` per disattivare una
di questefunzioni a livello globale. In ogni caso bisogna passare un valore per
entrambi i parametri, che sarà memorizzato in caso di una riattivazione successiva
della funzionalità disabilitata.

\param baud velocità della comunicazione seriale in baud
\param pinLed pin a cui è connesso il Led destinato al debug (numerazione dei pin
di Arduino)
*/
void Debug::inizializza(long baud, byte pinLed)
{
    _pinLed = pinLed;
    _baudComunicazioneSeriale = baud;

    //prepara il LED
    if(_usaLed)  pinMode(_pinLed, OUTPUT);
    //prepara Serial
    if(_usaHardwareSerial) Debug::serialBegin(_baudComunicazioneSeriale);

}

/**
Con "messaggio" si intende qualsiasi tipo di notifica a parte gli errori. Sono
quindi messaggi:
- i checkpoints
- i risultati finali o parziali dei calcoli svolti nel programma
- segnalazioni di qualsiasi tipo di input esterno

I messaggi, come gli errori, sono rappresentati da un numero (definito in
un'apposita tabella di `#define`). Al messaggio può essere associato un codice:
ad esempio si potrebbe avere il messaggio "calcolo la radice quadrata di" con
codice "16", e poi il messaggio "risultato" con codice "4"; oppure l'errore
"la funzione func() non ha restituito 0 ma..." con codice "3".

\warning Se si passa il valore `0` come codice sul monitor seriale esso non sarà
visibile: "errore 4, codice 0" sarà rappresentato come "errore 4, <nessun codice>".

Inoltre, se non si è selezionata la modalità "stampaMinimo", accanto a ogni messaggio
o errore apparirà il momento (in ms dall'inizio del programma) in cui si è verificato.

Se in un dato punto non è importante che il programma non venga rallentato più
del minimo necessario e per qualche motivo si vuole vedere ogni segnale del LED
singolarmente, si può impostare su `true`il terzo parametro.

\param numero Il numero che rappresenta il messaggio
\param codice [opzionale] Il codice/numero/... associato a quel messaggio
\param aspettaFineNotifica [opzionale] Blocca il programma fino a che il LED è spento
*/
void Debug::messaggio(int numero, long codice, bool aspettaFineNotifica) {

    Debug::accendiLed(_durataLuceMessaggio);


    //esci dalla la funzione se non si vuole che siano stampati i messaggi comuni
    if (!_stampaMessaggi)
    return;

    if(_usaHardwareSerial) {

        if(_stampaMinimo) {

            _hardwareSerial.print(numero);      //stampa il nr. che rappresenta il messaggio

            if (codice && !_ignoraCodice) {
                _hardwareSerial.print(S_SEP_NR_COD);
                _hardwareSerial.print(codice);  //ev. stampa il codice
            }

            _hardwareSerial.print("\n");        //vai a capo
        }

        else { //cioé if !_stampaMinimo

            _hardwareSerial.print(millis());   //stampa il tempo
            _hardwareSerial.print(S_SEP_T_NR);

            _hardwareSerial.print(S_MESS);       
            _hardwareSerial.print(numero);     //stampa il nr. che rappresenta il messaggio

            if (codice) {
                _hardwareSerial.print(S_SEP_NR_COD);
                _hardwareSerial.print(codice);  //ev. stampa il codice
            }

            _hardwareSerial.print("\n");       //vai a capo
        }
    }


    if(_aspettaFineNotifica || aspettaFineNotifica)
    Debug::aspettaFineNotifica();
}



/**
Con questa funzione si possono segnalare tutti gli errori che avvengono nel
programma ma che non ne compromettono totalmente il funzionamento, ma anche gli
errori fatali nel caso in cui si voglia vedere il comportamento dopo l'errore.

I numeri che rappresentano gli errori sono distinti da quelli dei messaggi (gli
errori sono segnalati in modo differente sia dal LED sia sul monitor seriale),
quindi le due liste di `#define` (quella dei messaggi e quella degli errori)
iniziano entrambe da 1 e hanno una numerazione indipendente.

Per più informazioni sul significato di `numero` e `codice` cfr. la documentazione
della funzione `messaggio(int, long, bool)`.

Ancheper informazioni sul terzo parametro cfr. la documentazione della funzione
`messaggio(int, long)`.

\param numero Il numero che rappresenta il messaggio
\param codice [opzionale] Il codice/numero/... associato a quel messaggio
\param aspettaFineNotifica [opzionale] Blocca il programma fino a che il LED è spento
*/
void Debug::errore(int numero, long codice, bool aspettaFineNotifica) {

    Debug::accendiLed(_durataLuceErrore);


    if(_usaHardwareSerial) {

        if(_stampaMinimo) {

            _hardwareSerial.print(S_ERR_MIN);          //segnala che si tratta di un errore
            _hardwareSerial.print(numero);      //stampa il nr. che rappresenta l'errore

            if (codice && !_ignoraCodice) {
                _hardwareSerial.print(S_SEP_NR_COD);
                _hardwareSerial.print(codice);  //ev. stampa il codice
            }

            _hardwareSerial.print("\n");        //vai a capo
        }

        else { //cioé if !_stampaMinimo

            _hardwareSerial.print("\n"); //salta una riga

            _hardwareSerial.print(millis());   //stampa il tempo
            _hardwareSerial.print(S_SEP_T_NR);

            _hardwareSerial.print(S_ERR);   //scrivi che è un errore
            _hardwareSerial.print(numero);     //stampa il nr. che rappresenta il messaggio

            if (codice) {
                _hardwareSerial.print(S_SEP_NR_COD);
                _hardwareSerial.print(codice);  //ev. stampa il codice
            }

            _hardwareSerial.print("\n\n");       //vai a capo e lascia una riga vuota
        }
    }


    if(_aspettaFineNotifica || aspettaFineNotifica)
    Debug::aspettaFineNotifica();

}



/**
Questa funzione è fondamentale per l'utilizzo del LED da parte della classe Debug.
Le altre funzioni possono accendere la spia luminosa ma non spegnerla; questa
funzione si occupa quindi di controllare se è "ora" di spegnere il LED e se è il
caso lo spegne.

\warning Questa funzione deve essere chiamata regolarmente e frequentemente per
poter avere dei segnali luminosi della giusta durata, altrimenti la luce resta
accesa per un tempo arbitrario che non dipende dal tempo richiesto dalla funzione
che l'ha accesa (ad es `messaggio()`) ma dalla posizione della chiamata di questa
funzione nel codice.
*/
void Debug::controllaLed() {
    if (!_ledAcceso) {
        return;
    }
    if (_durataLuceLed == 0) {
        return;
    }
    if (_tempoAccensioneLed + _durataLuceLed < millis()) {
        Debug::spegniLed();
    }
}

#endif //#ifdef DEBUG_ABILITA
