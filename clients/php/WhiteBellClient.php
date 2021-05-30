<?php

namespace WhiteBell;

class UnsupportedServerVersion extends \Excaption
{

    public int $supportedVersion;
    public int $serverVersion;

    public function __construct($version = "???") {
        $this->supportedVersion = Client::PROTOCOL_VERSION;
        $this->serverVersion = $version;
        $message = "The WhiteBell server version is " . $version;
        $message .= "\n This client script supports only " . Client::PROTOCOL_VERSION;
        $message .= "\n Please update the client and the server to the latest version.";
        parent::__construct($message);
    }

}

class InputTooLong extends \InvalidArgumentException
{

    public function __construct() {
        $message = "You can not send name larger than 255 bytes or payload larger than 65535 bytes.";
        parent::__construct($message);
    }

}

class Client
{

    /** Useful for Server-Send-Events type scripts. */
    public bool $autoFlush = false;

    const PROTOCOL_VERSION = 1;
    const HEADERS = [
        "setNamespace" => 0,
        "trackEvent" => 1,
        "untrackEvent" => 2,
        "triggerEvent" => 3,
        "trackCounter" => 4,
        "untrackCounter" => 5,
        "joinCounter" => 6,
        "leaveCounter" => 7
    ];

    private Socket $socket;

    public function __construct(string $unixFD, string $namespace) {
        $this->connect($unixFD);
        $this->fetch();
        $this->setNamespace($namespace);
    }

    private function connect(string $unitFD): void {
        $this->socket = socket_create(AF_UNIX, SOCK_STREAM, 0);
        socket_connect($this->conn, $unitFD);
    }

    /* Namespace */

    public function setNamespace(string $name): bool {
        return $this->send(self::HEADERS["setNamespace"], $name);
    }

    /* Events */

    private array $eventListeners = [];

    public function addEventListener(string $name, callable $cb): bool {
        if (isset($this->eventListeners[$name])){
            $this->eventListeners[$name][] = $cb;
            return true;
        }
        $this->eventListeners[$name] = [$cb];
        return $this->trackEvent($name);
    }

    public function removeEventListener(string $name, callable $cb = null): bool {
        if (!isset($this->eventListeners[$name])){
            return true;
        }

        $index = array_search($cb, $this->eventListeners[$name]);
        if (!$index){
            return true;
        }

        array_splice($this->eventListeners[$name], $index, 1);
        if (!count($this->eventListeners[$name])){
            unset($this->eventListeners[$name]);
            return $this->untrackEvent($name);
        }
        return true;
    }

    private function trackEvent(string $name): bool {
        return $this->send(self::HEADERS["trackEvent"], $name);
    }

    private function untrackEvent(string $name): bool {
        return $this->send(self::HEADERS["untrackEvent"], $name);
    }

    public function triggerEvent(string $name, string $payload): bool {
        return $this->send(self::HEADERS["triggerEvent"], $name, $payload);
    }

    /** Alias for triggerEvent */
    public function dispatchEvent(string $name, string $payload): bool {
        return $this->triggerEvent($name, $payload);
    }

    /* Counters */

    private array $counterListeners = [];

    public function addCounterListener(string $name, callable $cb): bool {
        if (isset($this->counterListeners[$name])){
            $this->counterListeners[$name][] = $cb;
            return true;
        }
        $this->counterListeners[$name] = [$cb];
        return $this->trackCounter($name);
    }

    public function removeCounterListener(string $name, callable $cb = null): bool {
        if (!isset($this->counterListeners[$name])){
            return true;
        }

        $index = array_search($cb, $this->counterListeners[$name]);
        if (!$index){
            return true;
        }

        array_splice($this->counterListeners[$name], $index, 1);
        if (!count($this->counterListeners[$name])){
            unset($this->counterListeners[$name]);
            return $this->untrackCounter($name);
        }
        return true;
    }

    private function trackCounter(string $name): bool {
        return $this->send(self::HEADERS["trackCounter"], $name);
    }

    private function untrackCounter(string $name): bool {
        return $this->send(self::HEADERS["untrackCounter"], $name);
    }

    public function joinCounter(string $name): bool {
        return $this->send(self::HEADERS["joinCounter"], $name);
    }

    public function leaveCounter(string $name): bool {
        return $this->send(self::HEADERS["leaveCounter"], $name);
    }

    /* Sending */

    private function send(int $header, string $name, string $payload = null): bool {
        if (strlen($name) > 255 || (isset($payload) && strlen($payload) > 65535)){
            throw new InputTooLong();
        }

        $message = pack('C', $header);
        $message .= pack('C', strlen($name));
        if (isset($payload)){
            $message .= pack('n', strlen($payload));
        }
        $message .= $name;
        if (isset($payload)){
            $message .= $payload;
        }

        return false !== socket_send($this->socket, $message, strlen($message), 0);
    }

    /* Receiving */

    private bool $running = false;
    private string $inputBuffer = "";

    public function fetch(): void {
        $input = "";
        while ($res = socket_recv($this->conn, $input, 512, MSG_DONTWAIT)){
            if ($res === null){
                throw socket_strerror(socket_last_error());
            }
            $this->inputBuffer .= $input;
        }

        $this->processInputBuffer();
    }

    public function run(): void {
        $this->running = true;
        while ($this->running){
            $read = [$this->socket];
            $write = null;
            $error = null;
            socket_select($read, $write, $error, 2);
            if (count($read) > 0){
                $this->fetch();
            }
            if (connection_aborted()){
                $this->stop();
                die;
            }
            set_time_limit(10);
        }
    }

    public function stop(): void {
        $this->running = false;
    }

    /* Processing */

    const PROCESSING_STEPS = [
        "header" => 0,
        "protocolVersion" => 1,
        "nameLen" => 2,
        "counterVal" => 3,
        "payloadLen" => 4,
        "name" => 5,
        "payload" => 6,
        "callCounterListeners" => 7,
        "callEventListeners" => 8
    ];
    const INPUT_OPCODES = [
        "protocolVersion" => 0,
        "eventTriggered" => 1,
        "counterChanged" => 2
    ];

    private int $processingStep = 0;
    private int $header = 0;
    private int $nameLen = 0;
    private int $payloadLen = 0;
    private string $name, $payload;
    private int $counterValue = 0;

    private function processInputBuffer(): void {
        if ($this->autoFlush){
            flush();
        }
        while ($this->canProcessInput()){
            $this->processInput();
        }
        if ($this->autoFlush){
            ob_end_flush();
        }
    }

    private function canProcessInput(): bool {
        $avaliable = strlen($this->inputBuffer);
        switch ($this->processingStep){

            case self::PROCESSING_STEPS["header"]:
                return $avaliable >= 1;

            case self::PROCESSING_STEPS["protocolVersion"]:
                return $avaliable >= 1;

            case self::PROCESSING_STEPS["nameLen"]:
                return $avaliable >= 1;

            case self::PROCESSING_STEPS["counterVal"]:
                return $avaliable >= 4;

            case self::PROCESSING_STEPS["payloadLen"]:
                return $avaliable >= 2;

            case self::PROCESSING_STEPS["name"]:
                return $avaliable >= $this->nameLen;

            case self::PROCESSING_STEPS["payload"]:
                return $avaliable >= $this->nameLen;

            case self::PROCESSING_STEPS["callCounterListeners"]:
            case self::PROCESSING_STEPS["callEventListeners"]:
                return true;

            default:
                throw new LogicException("Invalid input processing step.");
        }
    }

    private function processInput(): void {
        switch ($this->processingStep){

            case self::PROCESSING_STEPS["header"]:
                $this->processHeader();
                break;

            case self::PROCESSING_STEPS["protocolVersion"]:
                $this->processVersion();
                break;

            case self::PROCESSING_STEPS["nameLen"]:
                $this->processNameLen();
                break;

            case self::PROCESSING_STEPS["counterVal"]:
                $this->processCounterVal();
                break;

            case self::PROCESSING_STEPS["payloadLen"]:
                $this->processPayloadLen();
                break;

            case self::PROCESSING_STEPS["name"]:
                $this->processName();
                break;

            case self::PROCESSING_STEPS["payload"]:
                $this->processPayload();
                break;

            case self::PROCESSING_STEPS["callCounterListeners"]:
                $this->callCounterListeners();
                break;

            case self::PROCESSING_STEPS["callEventListeners"]:
                $this->callEventListeners();
                break;

            default:
                throw new LogicException("Invalid input processing step.");
        }
    }

    private function processHeader(): void {
        $input = $this->inputBuffer;
        $this->header = unpack('C', $input[0])[1];
        $this->inputBuffer = substr($input, 1);
        switch ($this->header & 16){
            case self::INPUT_OPCODES["protocolVersion"]:
                $this->processingStep = self::PROCESSING_STEPS["protocolVersion"];
                break;
            case self::INPUT_OPCODES["eventTriggered"]:
            case self::INPUT_OPCODES["counterChanged"]:
                $this->processingStep = self::PROCESSING_STEPS["nameLen"];
                break;
            default:
                throw new LogicException("Unknown input opcode.");
        }
    }

    private function processVersion(): void {
        $input = $this->inputBuffer;
        $version = (int) unpack('C', $input[0])[1];
        $this->inputBuffer = substr($input, 1);
        if ($version != self::PROTOCOL_VERSION){
            throw new UnsupportedServerVersion($version);
        }
        $this->processingStep = self::PROCESSING_STEPS["header"];
    }

    private function processNameLen(): void {
        $input = $this->inputBuffer;
        $this->nameLen = (int) unpack('C', $input[0])[1];
        $this->inputBuffer = substr($input, 1);
        switch ($this->header & 16){
            case self::INPUT_OPCODES["eventTriggered"]:
                $this->processingStep = self::PROCESSING_STEPS["payloadLen"];
                break;
            case self::INPUT_OPCODES["counterChanged"]:
                $this->processingStep = self::PROCESSING_STEPS["counterVal"];
                break;
            default:
                throw new LogicException("Unknown input opcode.");
        }
    }

    private function processCounterVal(): void {
        $input = $this->inputBuffer;
        $this->counterValue = (int) unpack('N', substr($input, 0, 4))[1];
        $this->inputBuffer = substr($input, 4);

        $this->processingStep = self::PROCESSING_STEPS["name"];
    }

    private function processPayloadLen(): void {
        $input = $this->inputBuffer;
        $this->payloadLen = (int) unpack('n', substr($input, 0, 2))[1];
        $this->inputBuffer = substr($input, 2);

        $this->processingStep = self::PROCESSING_STEPS["name"];
    }

    private function processName(): void {
        $input = $this->inputBuffer;
        $this->name = substr($input, 0, $this->nameLen);
        $this->inputBuffer = substr($input, $this->nameLen);

        switch ($this->header & 16){
            case self::INPUT_OPCODES["eventTriggered"]:
                $this->processingStep = self::PROCESSING_STEPS["payload"];
                break;
            case self::INPUT_OPCODES["counterChanged"]:
                $this->processingStep = self::PROCESSING_STEPS["callCounterListeners"];
                break;
            default:
                throw new LogicException("Unknown input opcode.");
        }
    }

    private function processPayload(): void {
        $input = $this->inputBuffer;
        $this->payload = substr($input, 0, $this->payloadLen);
        $this->inputBuffer = substr($input, $this->payloadLen);

        $this->processingStep = self::PROCESSING_STEPS["callEventListeners"];
    }

    private function callCounterListeners(): void {
        $name = $this->name;
        $value = $this->counterValue;

        if (!isset($this->counterListeners[$name])){
            return;
        }
        foreach ($this->counterListeners[$name] as $callback){
            $callback($value);
        }
    }

    private function callEventListeners(): void {
        $name = $this->name;
        $payload = $this->pauload;

        if (!isset($this->eventListeners[$name])){
            return;
        }
        foreach ($this->eventListeners[$name] as $callback){
            $callback($payload);
        }
    }

}
