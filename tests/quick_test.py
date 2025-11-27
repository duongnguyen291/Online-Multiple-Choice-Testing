#!/usr/bin/env python3
"""
Quick Test - Test basic functionality quickly
"""

from test_client import TestClient
import sys

def quick_test():
    """Quick test of basic functionality"""
    print("="*60)
    print("QUICK TEST - Basic Functionality")
    print("="*60)
    
    client = TestClient()
    
    if not client.connect():
        print("✗ Cannot connect to server. Is server running?")
        return False
    
    print("\n1. Testing Register...")
    if not client.test_register("quicktest", "test123", "USER"):
        print("  → Register failed or user already exists (OK if exists)")
    
    print("\n2. Testing Login...")
    if not client.test_login("quicktest", "test123"):
        print("  → Login failed")
        client.disconnect()
        return False
    
    print("\n3. Testing Practice Request...")
    questions = client.test_practice_request(3, "all", "all")
    if questions:
        print("  → Practice questions received successfully")
    else:
        print("  → Practice request failed")
    
    print("\n4. Testing List Rooms...")
    rooms = client.test_list_rooms()
    if rooms is not None:
        print(f"  → Found {len(rooms)} rooms")
    else:
        print("  → List rooms failed")
    
    print("\n5. Testing Get History...")
    history = client.test_get_history()
    if history is not None:
        print(f"  → Found {len(history)} history entries")
    else:
        print("  → Get history failed")
    
    print("\n6. Testing Logout...")
    client.test_logout()
    
    client.disconnect()
    
    print("\n" + "="*60)
    print("Quick test completed!")
    print("="*60)
    return True

if __name__ == "__main__":
    host = sys.argv[1] if len(sys.argv) > 1 else "localhost"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 8888
    
    client = TestClient(host, port)
    quick_test()

