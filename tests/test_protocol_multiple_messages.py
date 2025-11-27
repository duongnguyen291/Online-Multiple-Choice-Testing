#!/usr/bin/env python3
"""
Unit Test: Multiple Messages Handling
Test việc server xử lý nhiều messages trong một lần nhận (theo quy định "Lặp lại từ bước 1")
"""

import socket
import struct
import json
import time
import threading
from test_client import (
    TestClient, 
    C2S_REGISTER, C2S_LOGIN, C2S_LIST_ROOMS, C2S_PRACTICE_REQUEST, C2S_GET_HISTORY,
    S2C_RESPONSE_OK, S2C_LOGIN_OK, S2C_ROOM_LIST, S2C_PRACTICE_QUESTIONS, S2C_HISTORY_DATA
)

def send_multiple_messages(sock, messages):
    """Gửi nhiều messages liên tiếp trong một lần"""
    for msg_type, payload in messages:
        payload_str = json.dumps(payload, ensure_ascii=False)
        payload_bytes = payload_str.encode('utf-8')
        payload_length = len(payload_bytes)
        
        header = struct.pack('!HI', msg_type, payload_length)
        sock.sendall(header + payload_bytes)

def test_multiple_messages_handling():
    """Test server xử lý nhiều messages trong buffer"""
    print("=" * 60)
    print("TEST: Multiple Messages Handling")
    print("=" * 60)
    
    client = TestClient()
    if not client.connect():
        print("✗ Failed to connect")
        return False
    
    # Test 1: Gửi 3 messages liên tiếp (register, login, list_rooms)
    print("\n[TEST 1] Sending 3 messages in quick succession...")
    
    # Message 1: Register
    payload1 = {"username": "multitest1", "password": "pass123", "role": "USER"}
    client.send_message(C2S_REGISTER, payload1)
    
    # Message 2: Login (sẽ fail vì chưa register xong, nhưng test protocol)
    payload2 = {"username": "multitest1", "password": "pass123"}
    client.send_message(C2S_LOGIN, payload2)
    
    # Message 3: List rooms
    payload3 = {"session_token": "dummy"}  # Will fail but test protocol
    client.send_message(C2S_LIST_ROOMS, payload3)
    
    # Nhận responses
    responses = []
    for i in range(3):
        msg_type, response = client.recv_message()
        if msg_type:
            responses.append((msg_type, response))
            print(f"  Response {i+1}: type={msg_type}")
        else:
            break
    
    print(f"✓ Received {len(responses)} responses")
    client.disconnect()
    
    # Test 2: Gửi messages với delay nhỏ
    print("\n[TEST 2] Sending messages with small delays...")
    client2 = TestClient()
    if not client2.connect():
        return False
    
    # Register first
    client2.test_register("multitest2", "pass123", "USER")
    time.sleep(0.1)
    
    # Then login
    client2.test_login("multitest2", "pass123")
    time.sleep(0.1)
    
    # Then list rooms
    client2.test_list_rooms()
    
    client2.disconnect()
    
    print("\n✓ Multiple messages handling test completed")
    return True

def test_burst_messages():
    """Test gửi nhiều messages cùng lúc (burst)"""
    print("\n" + "=" * 60)
    print("TEST: Burst Messages")
    print("=" * 60)
    
    client = TestClient()
    if not client.connect():
        return False
    
    # Register user first
    client.test_register("bursttest", "pass123", "USER")
    client.test_login("bursttest", "pass123")
    
    # Send 10 list_rooms requests liên tiếp
    print("\n[TEST] Sending 10 LIST_ROOMS requests in burst...")
    for i in range(10):
        payload = {"session_token": client.session_token}
        client.send_message(C2S_LIST_ROOMS, payload)
    
    # Nhận tất cả responses
    responses_received = 0
    start_time = time.time()
    timeout = 5.0
    
    while responses_received < 10 and (time.time() - start_time) < timeout:
        msg_type, response = client.recv_message()
        if msg_type == S2C_ROOM_LIST:
            responses_received += 1
        elif msg_type is None:
            time.sleep(0.01)  # Small delay
            continue
    
    print(f"✓ Received {responses_received}/10 responses")
    
    if responses_received == 10:
        print("✓ All burst messages handled correctly")
        client.disconnect()
        return True
    else:
        print(f"✗ Only received {responses_received}/10 responses")
        client.disconnect()
        return False

def test_mixed_message_types():
    """Test xử lý nhiều loại messages khác nhau"""
    print("\n" + "=" * 60)
    print("TEST: Mixed Message Types")
    print("=" * 60)
    
    client = TestClient()
    if not client.connect():
        return False
    
    # Setup: Register and login
    client.test_register("mixedtest", "pass123", "USER")
    client.test_login("mixedtest", "pass123")
    
    # Send mixed messages
    print("\n[TEST] Sending mixed message types...")
    
    # 1. Practice request
    payload1 = {
        "session_token": client.session_token,
        "num_questions": 3,
        "topic": "all",
        "difficulty": "all"
    }
    client.send_message(C2S_PRACTICE_REQUEST, payload1)
    
    # 2. List rooms
    payload2 = {"session_token": client.session_token}
    client.send_message(C2S_LIST_ROOMS, payload2)
    
    # 3. Get history
    payload3 = {"session_token": client.session_token}
    client.send_message(C2S_GET_HISTORY, payload3)
    
    # Receive all responses
    responses = []
    start_time = time.time()
    while len(responses) < 3 and (time.time() - start_time) < 5.0:
        msg_type, response = client.recv_message()
        if msg_type:
            responses.append(msg_type)
            print(f"  Received response type: {msg_type}")
        else:
            time.sleep(0.01)
    
    print(f"✓ Received {len(responses)}/3 responses")
    
    if len(responses) == 3:
        print("✓ Mixed message types handled correctly")
        client.disconnect()
        return True
    else:
        print(f"✗ Only received {len(responses)}/3 responses")
        client.disconnect()
        return False

def run_all_multiple_message_tests():
    """Run all multiple message tests"""
    print("\n" + "=" * 60)
    print("MULTIPLE MESSAGES HANDLING TEST SUITE")
    print("=" * 60)
    
    tests = [
        ("Multiple Messages Handling", test_multiple_messages_handling),
        ("Burst Messages", test_burst_messages),
        ("Mixed Message Types", test_mixed_message_types),
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"\n✗ {name} failed with exception: {e}")
            import traceback
            traceback.print_exc()
            results.append((name, False))
        time.sleep(0.5)
    
    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    for name, result in results:
        status = "✓ PASSED" if result else "✗ FAILED"
        print(f"{name:30s}: {status}")
    
    passed = sum(1 for _, r in results if r)
    total = len(results)
    print(f"\nTotal: {passed}/{total} tests passed")
    print("=" * 60)
    
    return all(r for _, r in results)

if __name__ == "__main__":
    run_all_multiple_message_tests()

