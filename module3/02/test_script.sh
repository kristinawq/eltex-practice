#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' 

cleanup() {
    killall broker_app 2>/dev/null
    ipcrm -q 12345 2>/dev/null
    sleep 0.1
}

echo -e "\n1. Обработка ошибок ввода"
cleanup
./broker_app 2>&1 | grep -q "Использование" && echo -e "${GREEN}PASS: Запуск без аргументов${NC}" || echo -e "${RED}FAIL${NC}"
./broker_app -x 2>&1 | grep -q "Использование" && echo -e "${GREEN}PASS: Неизвестный флаг${NC}" || echo -e "${RED}FAIL${NC}"
./broker_app -b extra 2>&1 | grep -q "Использование" && echo -e "${GREEN}PASS: Брокер с лишними аргументами${NC}" || echo -e "${RED}FAIL${NC}"
./broker_app -p 2>&1 | grep -q "Использование" && echo -e "${GREEN}PASS: Издатель без темы${NC}" || echo -e "${RED}FAIL${NC}"
./broker_app -s 2>&1 | grep -q "Использование" && echo -e "${GREEN}PASS: Подписчик без тем${NC}" || echo -e "${RED}FAIL${NC}"

echo -e "\n2. Защита от второго Брокера"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_main.log 2>&1 &
BROKER_PID=$!
sleep 0.5
stdbuf -o0 ./broker_app -b > /tmp/broker2.log 2>&1
grep -q "Брокер уже запущен" /tmp/broker2.log && echo -e "${GREEN}PASS: Второй брокер заблокирован${NC}" || echo -e "${RED}FAIL${NC}"
kill -INT $BROKER_PID 2>/dev/null
sleep 0.3

echo -e "\n3. Базовая передача (1 Издатель -> 1 Подписчик)"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_main.log 2>&1 &
BROKER_PID=$!
sleep 0.3
stdbuf -o0 ./broker_app -s weather > /tmp/sub1.log 2>&1 &
SUB_PID=$!
sleep 0.5 # Даем подписчику отправить subscribe
stdbuf -o0 ./broker_app -p weather > /dev/null 2>&1
sleep 1.5 # Ждем, пока издатель отработает, а брокер перешлет данные
COUNT=$(grep -c "Получено сообщение" /tmp/sub1.log)
[ "$COUNT" -eq 5 ] && echo -e "${GREEN}PASS: Подписчик получил 5 сообщений${NC}" || echo -e "${RED}FAIL: Получено $COUNT (ожидалось 5)${NC}"
kill -INT $BROKER_PID 2>/dev/null
kill -INT $SUB_PID 2>/dev/null
sleep 0.3

echo -e "\n4. Фильтрация тем (игнорирование чужих сообщений)"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_main.log 2>&1 &
BROKER_PID=$!
sleep 0.3
stdbuf -o0 ./broker_app -s sport > /tmp/sub_sport.log 2>&1 &
SUB_PID=$!
sleep 0.5
stdbuf -o0 ./broker_app -p weather > /dev/null 2>&1
sleep 1.5
COUNT=$(grep -c "Получено сообщение" /tmp/sub_sport.log)
[ "$COUNT" -eq 0 ] && echo -e "${GREEN}PASS: Подписчик проигнорировал чужую тему${NC}" || echo -e "${RED}FAIL: Подписчик получил чужие сообщения${NC}"
kill -INT $BROKER_PID 2>/dev/null
kill -INT $SUB_PID 2>/dev/null
sleep 0.3

echo -e "\n5 Подписчик на 2 темы"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_main.log 2>&1 &
BROKER_PID=$!
sleep 0.3
stdbuf -o0 ./broker_app -s weather sport > /tmp/sub_multi.log 2>&1 &
SUB_PID=$!
sleep 0.5
stdbuf -o0 ./broker_app -p weather > /dev/null 2>&1
stdbuf -o0 ./broker_app -p sport > /dev/null 2>&1
sleep 1.5
COUNT=$(grep -c "Получено сообщение" /tmp/sub_multi.log)
[ "$COUNT" -eq 10 ] && echo -e "${GREEN}PASS: Подписчик получил 10 сообщений (5+5)${NC}" || echo -e "${RED}FAIL: Получено $COUNT (ожидалось 10)${NC}"
kill -INT $BROKER_PID 2>/dev/null
kill -INT $SUB_PID 2>/dev/null
sleep 0.3

echo -e "\n6. Несколько Подписчиков на одну тему"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_main.log 2>&1 &
BROKER_PID=$!
sleep 0.3
stdbuf -o0 ./broker_app -s news > /tmp/sub_n1.log 2>&1 &
SUB1_PID=$!
stdbuf -o0 ./broker_app -s news > /tmp/sub_n2.log 2>&1 &
SUB2_PID=$!
sleep 0.5
stdbuf -o0 ./broker_app -p news > /dev/null 2>&1
sleep 1.5
C1=$(grep -c "Получено сообщение" /tmp/sub_n1.log)
C2=$(grep -c "Получено сообщение" /tmp/sub_n2.log)
[ "$C1" -eq 5 ] && [ "$C2" -eq 5 ] && echo -e "${GREEN}PASS: Оба подписчика получили по 5 сообщений${NC}" || echo -e "${RED}FAIL: Подписчик 1: $C1, Подписчик 2: $C2${NC}"
kill -INT $BROKER_PID 2>/dev/null
kill -INT $SUB1_PID 2>/dev/null
kill -INT $SUB2_PID 2>/dev/null
sleep 0.3

echo -e "\n7. Завершение по сигналу SIGINT (Ctrl+C)"
cleanup
stdbuf -o0 ./broker_app -b > /tmp/broker_sig.log 2>&1 &
BROKER_PID=$!
sleep 0.3
stdbuf -o0 ./broker_app -s test > /tmp/sub_sig.log 2>&1 &
SUB_PID=$!
sleep 0.5
kill -INT $BROKER_PID 
sleep 1.0
grep -q "Завершение работы" /tmp/broker_sig.log && echo -e "${GREEN}PASS: Брокер начал завершение${NC}" || echo -e "${RED}FAIL: Брокер не отреагировал${NC}"
grep -q "Получен сигнал завершения" /tmp/sub_sig.log && echo -e "${GREEN}PASS: Подписчик завершился по сигналу${NC}" || echo -e "${RED}FAIL: Подписчик не завершился${NC}"

cleanup
