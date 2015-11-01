# ------------------------------------------------------------------------------
#  binstruct unit testing
#
#  Thomas Bonim (thomas.bonim@googlemail.com)
#  This code is in the public domain
# ------------------------------------------------------------------------------
import unittest
import binstruct


class TestModule(unittest.TestCase):
    def test_error(self):
        with self.assertRaises(binstruct.error):
            raise binstruct.error('test')


class TestObject(unittest.TestCase):
    def setUp(self):
        self.data = b"\x11\x22\x00\x44\x55"
        self.obj = binstruct.Binstruct(self.data)

    def test_data(self):
        self.assertIs(self.obj.data, self.data)
        self.assertEqual(self.obj.pos, 0)
        with self.assertRaises(AttributeError):
            self.obj.data = "other"

    def test_pos(self):
        self.assertEqual(self.obj.pos, 0)
        self.assertIsInstance(self.obj.pos, int)
        self.obj.pos = 1
        self.assertEqual(self.obj.pos, 1)
        self.assertIsInstance(self.obj.pos, int)
        self.obj.pos = 2L
        self.assertEqual(self.obj.pos, 2)
        self.assertIsInstance(self.obj.pos, int)
        self.obj.pos += 1
        self.assertEqual(self.obj.pos, 3)
        self.assertIsInstance(self.obj.pos, int)
        self.obj.pos += 1L
        self.assertEqual(self.obj.pos, 4)
        self.assertIsInstance(self.obj.pos, int)
        with self.assertRaises(TypeError):
            self.obj.pos = None
        with self.assertRaises(TypeError):
            del self.obj.pos
        with self.assertRaises(binstruct.error):
            self.obj.pos = -1
        with self.assertRaises(binstruct.error):
            self.obj.pos = 6

    def test_init(self):
        data = 'other'
        self.obj.pos += 1
        self.obj.__init__(data)
        self.assertIs(self.obj.data, data)
        self.assertEqual(self.obj.pos, 0)


class TestEmpty(unittest.TestCase):
    def test_range_u8(self):
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_u8()
        with self.assertRaises(binstruct.error):
            obj.pos += 1


class TestUnpackByte(unittest.TestCase):
    def test_bool(self):
        obj = binstruct.Binstruct(b"\x00\x01")
        value = obj.unpack_bool()
        self.assertIsInstance(value, bool)
        self.assertEqual(value, False)
        self.assertEqual(obj.pos, 1)
        value = obj.unpack_bool()
        self.assertIsInstance(value, bool)
        self.assertEqual(value, True)
        self.assertEqual(obj.pos, 2)
        with self.assertRaises(binstruct.error):
            obj.unpack_bool()

    def test_u8(self):
        obj = binstruct.Binstruct(b"\x81\x82")
        with self.assertRaises(TypeError):
            obj.unpack_u8(None)

        value = obj.unpack_u8()
        self.assertEqual(value, 0x81)
        self.assertEqual(obj.pos, 1)
        self.assertIsInstance(value, int)
        self.assertEqual(obj.unpack_u8(), 0x82)
        self.assertEqual(obj.pos, 2)

    def test_s8(self):
        obj = binstruct.Binstruct(b"\x81\x82")
        value = obj.unpack_s8()
        self.assertEqual(value, -127)
        self.assertEqual(obj.pos, 1)
        self.assertIsInstance(value, int)
        value = obj.unpack_s8()
        self.assertEqual(value, -126)
        self.assertEqual(obj.pos, 2)
        self.assertIsInstance(value, int)
        with self.assertRaises(TypeError):
            obj.unpack_s8(None)


class TestUnpackBE(unittest.TestCase):
    def setUp(self):
        self.obj = binstruct.Binstruct(b"\x81\x82\x83\x84\x85\x86\x87\x88")

    def test_ube16(self):
        value = self.obj.unpack_ube16()
        self.assertEqual(value, 0x8182)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 2)

    def test_sbe16(self):
        value = self.obj.unpack_sbe16()
        self.assertEqual(value, -32382)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 2)

    def test_ube32(self):
        value = self.obj.unpack_ube32()
        self.assertEqual(value, 0x81828384)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 4)

    def test_sbe32(self):
        value = self.obj.unpack_sbe32()
        self.assertEqual(value, -2122153084)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 4)

    def test_ube64(self):
        value = self.obj.unpack_ube64()
        self.assertEqual(value, 0x8182838485868788)
        self.assertIsInstance(value, long)
        self.assertEqual(self.obj.pos, 8)

    def test_sbe64(self):
        value = self.obj.unpack_sbe64()
        self.assertEqual(value, -9114578090645354616)
        self.assertIsInstance(value, long)
        self.assertEqual(self.obj.pos, 8)


class TestUnpackLE(unittest.TestCase):
    def setUp(self):
        self.obj = binstruct.Binstruct(b"\x81\x82\x83\x84\x85\x86\x87\x88")

    def test_ule16(self):
        value = self.obj.unpack_ule16()
        self.assertEqual(value, 0x8281)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 2)

    def test_sle16(self):
        value = self.obj.unpack_sle16()
        self.assertEqual(value, -32127)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 2)

    def test_ule32(self):
        value = self.obj.unpack_ule32()
        self.assertEqual(value, 0x84838281)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 4)

    def test_sle32(self):
        value = self.obj.unpack_sle32()
        self.assertEqual(value, -2071756159)
        self.assertIsInstance(value, int)
        self.assertEqual(self.obj.pos, 4)

    def test_ule64(self):
        value = self.obj.unpack_ule64()
        self.assertEqual(value, 0x8887868584838281)
        self.assertIsInstance(value, long)
        self.assertEqual(self.obj.pos, 8)

    def test_sle64(self):
        value = self.obj.unpack_sle64()
        self.assertEqual(value, -8608764254683430271)
        self.assertIsInstance(value, long)
        self.assertEqual(self.obj.pos, 8)


class TestUnpackULEB128(unittest.TestCase):
    def test_arg(self):
        obj = binstruct.Binstruct(b"\01")
        with self.assertRaises(TypeError):
            obj.unpack_uleb128(None)

    def test_value(self):
        obj = binstruct.Binstruct(b"\xE5\x8E\x26\x00\xFF")
        value = obj.unpack_uleb128()
        self.assertIsInstance(value, int)
        self.assertEqual(value, 624485)
        self.assertEqual(obj.pos, 3)

    def test_long(self):
        obj = binstruct.Binstruct(b"\xFF\xFF\xFF\xFF\xFF\xE5\x8E\x26\x00\xFF")
        value = obj.unpack_uleb128()
        self.assertIsInstance(value, long)
        self.assertEqual(value, 18446744073709551615L)
        self.assertEqual(obj.pos, 8)

    def test_empty(self):
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_uleb128()

    def test_zero(self):
        obj = binstruct.Binstruct(b"\x00")
        self.assertEqual(obj.unpack_uleb128(), 0)
        self.assertEqual(obj.pos, 1)

    def test_corrupt(self):
        obj = binstruct.Binstruct(b"\x80")
        with self.assertRaises(binstruct.error):
            obj.unpack_uleb128()

    def test_messy(self):
        obj = binstruct.Binstruct(b"\x80\x01")
        self.assertEqual(obj.unpack_uleb128(), 0x80)
        self.assertEqual(obj.pos, 2)


class TestUnpackSLEB128(unittest.TestCase):
    def test_arg(self):
        obj = binstruct.Binstruct(b"\01")
        with self.assertRaises(TypeError):
            obj.unpack_sleb128(None)

    def test_value(self):
        obj = binstruct.Binstruct(b"\x9b\xf1\x59\x00\xFF")
        value = obj.unpack_sleb128()
        self.assertIsInstance(value, int)
        self.assertEqual(value, -624485)
        self.assertEqual(obj.pos, 3)

    def test_long(self):
        obj = binstruct.Binstruct(b"\xFF\xFF\xFF\xE5\x8E\x76\x00\xFF")
        value = obj.unpack_sleb128()
        self.assertIsInstance(value, int)
        self.assertEqual(value, -322961409)
        self.assertEqual(obj.pos, 6)

    def test_empty(self):
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_sleb128()

    def test_zero(self):
        obj = binstruct.Binstruct(b"\x00")
        self.assertEqual(obj.unpack_sleb128(), 0)
        self.assertEqual(obj.pos, 1)

    def test_corrupt(self):
        obj = binstruct.Binstruct(b"\x80")
        with self.assertRaises(binstruct.error):
            obj.unpack_sleb128()

    def test_messy(self):
        obj = binstruct.Binstruct(b"\x80\x01")
        self.assertEqual(obj.unpack_sleb128(), 0x80)
        self.assertEqual(obj.pos, 2)


class TestUnpackString(unittest.TestCase):
    def test_string(self):
        obj = binstruct.Binstruct(b"test\x00next\x00")
        value = obj.unpack_string()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "test")
        self.assertEqual(obj.pos, 5)
        value = obj.unpack_string()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "next")
        self.assertEqual(obj.pos, 10)

    def test_empty(self):
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_string()

    def test_unterminated(self):
        obj = binstruct.Binstruct(b"test")
        value = obj.unpack_string()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "test")

    def test_arg(self):
        obj = binstruct.Binstruct(b"\01")
        with self.assertRaises(TypeError):
            obj.unpack_string(None)


class TestUnpackBlock(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"test\x11")
        value = obj.unpack_block(4)
        self.assertIsInstance(value, str)
        self.assertEqual(value, b"test")
        self.assertEqual(obj.pos, 4)
        value = obj.unpack_block(1)
        self.assertIsInstance(value, str)
        self.assertEqual(value, b"\x11")
        self.assertEqual(obj.pos, 5)
        value = obj.unpack_block(0)
        self.assertIsInstance(value, str)
        self.assertEqual(value, b"")
        self.assertEqual(obj.pos, 5)

    def test_error(self):
        data = "test\x11"
        obj = binstruct.Binstruct(data)
        with self.assertRaises(TypeError):
            obj.unpack_block()
        with self.assertRaises(TypeError):
            obj.unpack_block(None)
        with self.assertRaises(binstruct.error):
            obj.unpack_block(-1)
        with self.assertRaises(binstruct.error):
            obj.unpack_block(6)
        obj.pos = 5
        with self.assertRaises(binstruct.error):
            obj.unpack_block(1)
        self.assertEqual(obj.pos, 5)


class TestUnpackBlock8(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"\x03test")
        value = obj.unpack_block_u8()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 4)
        obj = binstruct.Binstruct(b"\x00test")
        value = obj.unpack_block_u8()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 1)

    def test_error(self):
        obj = binstruct.Binstruct(b"\x05test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_u8()
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_u8()
        obj = binstruct.Binstruct(b"test")
        with self.assertRaises(TypeError):
            obj.unpack_block_u8(None)


class TestUnpackBlock16(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"\x00\x03test")
        value = obj.unpack_block_be16()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 5)
        obj = binstruct.Binstruct(b"\x03\x00test")
        value = obj.unpack_block_le16()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 5)
        obj = binstruct.Binstruct(b"\x00\x00test")
        value = obj.unpack_block_be16()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 2)
        obj = binstruct.Binstruct(b"\x00\x00test")
        value = obj.unpack_block_le16()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 2)

    def test_error(self):
        obj = binstruct.Binstruct(b"\x05\x00test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be16()
        obj = binstruct.Binstruct(b"\x00\x05test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le16()
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be16()
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le16()
        obj = binstruct.Binstruct(b"test")
        with self.assertRaises(TypeError):
            obj.unpack_block_be16(None)
        with self.assertRaises(TypeError):
            obj.unpack_block_le16(None)


class TestUnpackBlock32(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"\x00\x00\x00\x03test")
        value = obj.unpack_block_be32()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 7)
        obj = binstruct.Binstruct(b"\x03\x00\x00\x00test")
        value = obj.unpack_block_le32()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 7)
        obj = binstruct.Binstruct(b"\x00\x00\x00\x00test")
        value = obj.unpack_block_be32()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 4)
        obj = binstruct.Binstruct(b"\x00\x00\x00\x00test")
        value = obj.unpack_block_le32()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 4)

    def test_error(self):
        obj = binstruct.Binstruct(b"\x05\x00\x00\x00test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be32()
        obj = binstruct.Binstruct(b"\x00\x00\x00\x05test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le32()
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be32()
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le32()
        obj = binstruct.Binstruct(b"test")
        with self.assertRaises(TypeError):
            obj.unpack_block_be32(None)
        with self.assertRaises(TypeError):
            obj.unpack_block_le32(None)


class TestUnpackBlock64(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"\x00\x00\x00\x00\x00\x00\x00\x03test")
        value = obj.unpack_block_be64()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 11)
        obj = binstruct.Binstruct(b"\x03\x00\x00\x00\x00\x00\x00\x00test")
        value = obj.unpack_block_le64()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 11)
        obj = binstruct.Binstruct(b"\x00\x00\x00\x00\x00\x00\x00\x00test")
        value = obj.unpack_block_be64()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 8)
        obj = binstruct.Binstruct(b"\x00\x00\x00\x00\x00\x00\x00\x00test")
        value = obj.unpack_block_le64()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "")
        self.assertEqual(obj.pos, 8)

    def test_error(self):
        obj = binstruct.Binstruct(b"\x05\x00\x00\x00\x00\x00\x00\x00test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be64()
        obj = binstruct.Binstruct(b"\x00\x00\x00\x05\x00\x00\x00\x00test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le64()
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_be64()
        with self.assertRaises(binstruct.error):
            obj.unpack_block_le64()
        obj = binstruct.Binstruct(b"test")
        with self.assertRaises(TypeError):
            obj.unpack_block_be64(None)
        with self.assertRaises(TypeError):
            obj.unpack_block_le64(None)


class TestUnpackBlockULEB128(unittest.TestCase):
    def test_block(self):
        obj = binstruct.Binstruct(b"\x03test")
        value = obj.unpack_block_uleb128()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "tes")
        self.assertEqual(obj.pos, 4)
        obj = binstruct.Binstruct(b"\x84\x80\x80\x00test")
        value = obj.unpack_block_uleb128()
        self.assertIsInstance(value, str)
        self.assertEqual(value, "test")
        self.assertEqual(obj.pos, 8)

    def test_error(self):
        obj = binstruct.Binstruct(b"\x05test")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_uleb128()
        obj = binstruct.Binstruct(b"")
        with self.assertRaises(binstruct.error):
            obj.unpack_block_uleb128()
        obj = binstruct.Binstruct(b"test")
        with self.assertRaises(TypeError):
            obj.unpack_block_uleb128(None)


if __name__ == '__main__':
    unittest.main()
