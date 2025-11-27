#!/usr/bin/env python3
"""
Individual Test Cases - Test each functionality separately
"""

from test_client import TestClient
import time

def test_authentication():
    """Test authentication flow"""
    print("\n" + "="*60)
    print("TEST SUITE: Authentication")
    print("="*60)
    
    client = TestClient()
    if not client.connect():
        return False
    
    # Test register
    print("\n1. Testing Register...")
    assert client.test_register("newuser", "pass123", "USER"), "Register failed"
    
    # Test duplicate register (should fail)
    print("\n2. Testing Duplicate Register (should fail)...")
    result = client.test_register("newuser", "pass123", "USER")
    assert not result, "Duplicate register should fail"
    
    # Test login
    print("\n3. Testing Login...")
    assert client.test_login("newuser", "pass123"), "Login failed"
    
    # Test invalid login
    print("\n4. Testing Invalid Login (should fail)...")
    result = client.test_login("newuser", "wrongpass")
    assert not result, "Invalid login should fail"
    
    # Retry valid login
    assert client.test_login("newuser", "pass123"), "Valid login failed"
    
    # Test logout
    print("\n5. Testing Logout...")
    assert client.test_logout(), "Logout failed"
    
    client.disconnect()
    print("\n✓ Authentication tests passed!")
    return True


def test_practice_mode():
    """Test practice mode"""
    print("\n" + "="*60)
    print("TEST SUITE: Practice Mode")
    print("="*60)
    
    client = TestClient()
    if not client.connect():
        return False
    
    # Login
    assert client.test_login("newuser", "pass123"), "Login failed"
    
    # Request practice questions
    print("\n1. Testing Practice Request...")
    questions = client.test_practice_request(5, "all", "all")
    assert questions and len(questions) > 0, "No questions received"
    
    # Submit practice
    print("\n2. Testing Practice Submit...")
    answers = []
    for q in questions:
        answers.append({
            "q_id": q.get("q_id"),
            "selected_option": "option_a"
        })
    assert client.test_practice_submit(answers), "Practice submit failed"
    
    # Test with filters
    print("\n3. Testing Practice Request with Filters...")
    questions = client.test_practice_request(3, "Toán học", "easy")
    assert questions is not None, "Filtered practice request failed"
    
    client.disconnect()
    print("\n✓ Practice mode tests passed!")
    return True


def test_test_rooms():
    """Test test room functionality"""
    print("\n" + "="*60)
    print("TEST SUITE: Test Rooms")
    print("="*60)
    
    # Setup: Create teacher and user
    teacher_client = TestClient()
    user_client = TestClient()
    
    if not teacher_client.connect() or not user_client.connect():
        return False
    
    # Register and login teacher
    teacher_client.test_register("teacher1", "pass123", "TEACHER")
    assert teacher_client.test_login("teacher1", "pass123"), "Teacher login failed"
    
    # Register and login user
    user_client.test_register("student1", "pass123", "USER")
    assert user_client.test_login("student1", "pass123"), "User login failed"
    
    # Test create room
    print("\n1. Testing Create Room (Teacher)...")
    room_id = teacher_client.test_create_room("Math Test", 10, 30, "Toán học", "easy")
    assert room_id is not None, "Create room failed"
    
    # Test list rooms
    print("\n2. Testing List Rooms...")
    rooms = user_client.test_list_rooms()
    assert rooms is not None, "List rooms failed"
    assert any(r.get("room_id") == room_id for r in rooms), "Created room not in list"
    
    # Test join room
    print("\n3. Testing Join Room...")
    assert user_client.test_join_room(room_id), "Join room failed"
    
    # Test user cannot create room
    print("\n4. Testing User Cannot Create Room...")
    result = user_client.test_create_room("User Room", 5, 15, "all", "all")
    assert result is None, "User should not be able to create room"
    
    teacher_client.disconnect()
    user_client.disconnect()
    print("\n✓ Test room tests passed!")
    return True


def test_history():
    """Test history functionality"""
    print("\n" + "="*60)
    print("TEST SUITE: History")
    print("="*60)
    
    client = TestClient()
    if not client.connect():
        return False
    
    # Login
    assert client.test_login("newuser", "pass123"), "Login failed"
    
    # Get history
    print("\n1. Testing Get History...")
    history = client.test_get_history()
    assert history is not None, "Get history failed"
    
    client.disconnect()
    print("\n✓ History tests passed!")
    return True


def run_all_individual_tests():
    """Run all individual test suites"""
    print("\n" + "="*60)
    print("INDIVIDUAL TEST SUITES")
    print("="*60)
    
    tests = [
        ("Authentication", test_authentication),
        ("Practice Mode", test_practice_mode),
        ("Test Rooms", test_test_rooms),
        ("History", test_history),
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"\n✗ {name} test suite failed with exception: {e}")
            results.append((name, False))
        time.sleep(0.5)  # Small delay between test suites
    
    # Summary
    print("\n" + "="*60)
    print("TEST SUMMARY")
    print("="*60)
    for name, result in results:
        status = "✓ PASSED" if result else "✗ FAILED"
        print(f"{name:20s}: {status}")
    
    passed = sum(1 for _, r in results if r)
    total = len(results)
    print(f"\nTotal: {passed}/{total} test suites passed")
    print("="*60)
    
    return all(r for _, r in results)


if __name__ == "__main__":
    run_all_individual_tests()

