Soft TCAM
=========

TCAM Emulation Library

## これはなに？

TCAM っぽい機能を実現するライブラリです。

## どうやって使うの？

以下のように使います。

    #include <cstdint>
    #include <bitset>
    #include <iostream>

    #include "soft_tcam.h"

    int
    main(int argc, char *argv[])
    {
    	soft_tcam::soft_tcam<std::uint32_t, 32> tcam;
    	std::bitset<32> data, mask, key;
    	std::uint32_t priority;
    	const std::uint32_t *result;
    
    	data		= 0x12340000;
    	mask		= 0xffff0000;
    	priority	= 0x00000010;
    	tcam.insert(data, mask, priority, 1);
    
    	data		= 0x12000000;
    	mask		= 0xff000000;
    	priority	= 0x00000008;
    	tcam.insert(data, mask, priority, 2);
    
    	key 		= 0x12345678;
    	result = tcam.find(key);
    	std::cout << "Find result: ";
    	if (result != nullptr) {
    		std::cout << *result;
    	} else {
    		std::cout << "nullptr";
    	}
    	std::cout << std::endl;
    
    	data		= 0x12340000;
    	mask		= 0xffff0000;
    	priority	= 0x00000010;
    	tcam.erase(data, mask, priority, 1);
    
    	data		= 0x12000000;
    	mask		= 0xff000000;
    	priority	= 0x00000008;
    	tcam.erase(data, mask, priority, 2);
    
    	return 0;
    }

## もっと詳しく？

    soft_tcam::soft_tcam<std::uint32_t, 32> tcam;

`tcam` という名前の Soft TCAM オブジェクトを作成します。

Soft TCAM は作成時に 2 つのテンプレート引数を指定します。

ひとつめは Soft TCAM が保持するデータ型です。上記の例では `std::uint32_t` を指定しているので `tcam` に格納するデータは 32 ビット符号無し整数です。

ふたつめは Soft TCAM のビット長です。上記の例では `32` を指定しているので `tcam` には 32 ビットのデータとマスクを指定してデータを格納することができます。

    tcam.insert(data, mask, priority, 1);

Soft TCAM にデータを格納するには `insert()` メンバ関数を呼び出します。

`insert()` メンバ関数は 4 つの引数を取ります。

ひとつめは探索時のキーとなるデータです。これは Soft TCAM 変数作成時に指定したのと同じビット長の `std::bitset` 型になります。

ふたつめは探索時のキーとなるマスクです。これも Soft TCAM 変数作成時に指定したのと同じビット長の `std::bitset` 型になります。

みっつめはプライオリティ（優先度）です。探索時に複数マッチするデータ＆マスクが存在した場合この値が最も大きいものが結果として返されます。

よっつめは格納するデータです。これは Soft TCAM 変数作成時に指定したデータ型（ひとつめのテンプレート引数で指定した型）の変数を指定します。通常はなんらかのデータ型へのポインタを格納する感じになるんじゃないでしょうか。

正常に格納することができときは `0` を返します。

マスクが `1` なのにデータも `1` になっているビットが存在するなど正常に格納できなかったときは `-1` を返します。

    result = tcam.find(key);

`find()` メンバ関数はあるキー値にマッチするプライオリティが最も大きいエントリーを探索する関数です。

引数は Soft TCAM 変数作成時に指定したのと同じビット長の `std::bitset` 型の変数を指定します。

該当するエントリーが存在した場合はそのエントリーへのポインタが返されます。該当するエントリーが存在しない場合は `nullptr` が返されます。

    tcam.erase(data, mask, priority, 1);

`erase()` メンバ関数は `insert()` で登録したエントリーを削除します。引数は `insert()` と同様です。

正常に削除することができたときは `0` を返します。

該当するエントリーが存在しないなど正常に削除できなかったときは `-1` を返します。
